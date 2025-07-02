#include "VoxelWorld.h"
#include "VoxelChunk.h"
#include "VoxelBaseDataAsset.h"
#include "VoxelBlockDataAsset.h"
#include "VoxelSubstanceDataAsset.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

AVoxelWorld::AVoxelWorld()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AVoxelWorld::InitializeWorld(int32 NumberOfPlayers)
{
    VoxelDataMap.Empty();
    for (int32 i = 0; i < VoxelDataAssets.Num(); ++i)
    {
        if (VoxelDataAssets[i]) VoxelDataMap.Add(i, VoxelDataAssets[i]);
    }

    bool bFoundVoid = false, bFoundSolid = false;
    for (const auto& Pair : VoxelDataMap)
    {
        if (Pair.Value == VoidVoxelData) { VoidID = Pair.Key; bFoundVoid = true; }
        if (Pair.Value == DefaultVoxelData) { DefaultID = Pair.Key; bFoundSolid = true; }
    }

    if (!bFoundVoid) UE_LOG(LogTemp, Warning, TEXT("VoidVoxelData is not set or not found!"));
    if (!bFoundSolid) UE_LOG(LogTemp, Warning, TEXT("DefaultVoxelData is not set or not found!"));

    WorldSize = WorldSizeInChunks * CHUNK_SIZE;
    WorldVolume = WorldSize.X * WorldSize.Y * WorldSize.Z;

    VoxelIDs.SetNumZeroed(WorldVolume);
    Durabilities.SetNumZeroed(WorldVolume);

    for (int32 i = 0; i < WorldVolume; ++i)
    {
        VoxelIDs[i] = DefaultID;
        Durabilities[i] = 100;
    }

    GenerateRooms(NumberOfPlayers);

    for (int32 x = 0; x < WorldSizeInChunks.X; ++x)
    {
        for (int32 y = 0; y < WorldSizeInChunks.Y; ++y)
        {
            for (int32 z = 0; z < WorldSizeInChunks.Z; ++z)
            {
                SpawnChunk(FIntVector(x, y, z));
            }
        }
    }
}

void AVoxelWorld::GenerateRooms(int32 NumberOfRoomsToGenerate)
{
    GeneratedRooms.Empty();
    for (APlayerStart* StartPoint : PlayerStartPoints)
    {
        if (StartPoint) StartPoint->Destroy();
    }
    PlayerStartPoints.Empty();

    const FVector WorldMaxBounds = FVector(WorldSize) * VOXEL_SIZE;
    const FVector WorldMinBounds = FVector::ZeroVector;

    const int32 MaxPlacementAttempts = 100;

    for (int32 i = 0; i < NumberOfRoomsToGenerate; ++i)
    {
        FSphericalRoom NewRoom;
        NewRoom.Radius = RoomRadius;

        const FVector SafeMinBounds = WorldMinBounds + NewRoom.Radius;
        const FVector SafeMaxBounds = WorldMaxBounds - NewRoom.Radius;

        if (SafeMinBounds.X >= SafeMaxBounds.X || SafeMinBounds.Y >= SafeMaxBounds.Y || SafeMinBounds.Z >= SafeMaxBounds.Z)
        {
            UE_LOG(LogTemp, Warning, TEXT("Room radius is too large for the world size. Skipping room generation for room %d."), i);
            continue;
        }

        bool bRoomPlaced = false;
        for (int32 Attempt = 0; Attempt < MaxPlacementAttempts; ++Attempt)
        {
            FVector CenterLocation = FVector(
                FMath::RandRange(SafeMinBounds.X, SafeMaxBounds.X),
                FMath::RandRange(SafeMinBounds.Y, SafeMaxBounds.Y),
                FMath::RandRange(SafeMinBounds.Z, SafeMaxBounds.Z)
            );
            NewRoom.Center = CenterLocation;

            bool bOverlaps = false;
            for (const FSphericalRoom& ExistingRoom : GeneratedRooms)
            {
                float DistanceBetweenCenters = FVector::Dist(NewRoom.Center, ExistingRoom.Center);
                if (DistanceBetweenCenters < (NewRoom.Radius + ExistingRoom.Radius))
                {
                    bOverlaps = true;
                    break;
                }
            }

            if (!bOverlaps)
            {
                GeneratedRooms.Add(NewRoom);
                bRoomPlaced = true;
                break;
            }
        }

        if (!bRoomPlaced)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to place room %d after %d attempts. Skipping."), i, MaxPlacementAttempts);
            continue;
        }

        FIntVector MinAffectedGrid = FIntVector(FVector(NewRoom.Center - NewRoom.Radius) / VOXEL_SIZE);
        FIntVector MaxAffectedGrid = FIntVector(FVector(NewRoom.Center + NewRoom.Radius) / VOXEL_SIZE);

        for (int32 z = FMath::Max(0, MinAffectedGrid.Z); z < FMath::Min(WorldSize.Z, MaxAffectedGrid.Z + 1); ++z)
        {
            for (int32 y = FMath::Max(0, MinAffectedGrid.Y); y < FMath::Min(WorldSize.Y, MaxAffectedGrid.Y + 1); ++y)
            {
                for (int32 x = FMath::Max(0, MinAffectedGrid.X); x < FMath::Min(WorldSize.X, MaxAffectedGrid.X + 1); ++x)
                {
                    FIntVector GlobalBlockCoord(x, y, z);
                    FVector WorldPosOfBlock = FVector(GlobalBlockCoord) * VOXEL_SIZE;

                    if (FVector::DistSquared(WorldPosOfBlock, NewRoom.Center) < FMath::Square(NewRoom.Radius))
                    {
                        SetVoxelID(GlobalBlockCoord, VoidID);
                        SetDurability(GlobalBlockCoord, 0.0f);
                    }
                }
            }
        }

        FTransform SpawnTransform(FRotator::ZeroRotator, NewRoom.Center);
        APlayerStart* NewPlayerStart = GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), SpawnTransform);
        if (NewPlayerStart)
        {
            PlayerStartPoints.Add(NewPlayerStart);
            UE_LOG(LogTemp, Log, TEXT("Generated PlayerStart at: %s"), *NewPlayerStart->GetActorLocation().ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn PlayerStart for room %d at %s"), i, *NewRoom.Center.ToString());
        }
    }
    UE_LOG(LogTemp, Log, TEXT("VoxelWorld: Generated %d PlayerStarts."), PlayerStartPoints.Num());
}

void AVoxelWorld::SpawnChunk(const FIntVector& ChunkCoord)
{
    if (!Chunks.Contains(ChunkCoord))
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        FVector ChunkWorldLocation = FVector(ChunkCoord) * CHUNK_SIZE * VOXEL_SIZE;
        AVoxelChunk* NewChunk = GetWorld()->SpawnActor<AVoxelChunk>(ChunkWorldLocation, GetActorRotation(), SpawnParams);
        if (NewChunk)
        {
            NewChunk->Initialize(this, ChunkCoord);
            NewChunk->GenerateMesh();
            Chunks.Add(ChunkCoord, NewChunk);

            NewChunk->DrawDebugBounds(1.f, FColor::Green);
        }
    }
}

UVoxelBaseDataAsset* AVoxelWorld::GetVoxelData(int32 VoxelID) const
{
    if (const TObjectPtr<UVoxelBaseDataAsset>* FoundData = VoxelDataMap.Find(VoxelID)) return *FoundData;
    return VoxelDataMap.FindRef(VoidID);
}

UMaterialInterface* AVoxelWorld::GetVoxelMaterial(int32 VoxelID) const
{
    if (const UVoxelBaseDataAsset* VoxelData = GetVoxelData(VoxelID))
    {
        if (const UVoxelSubstanceDataAsset* SubstanceData = Cast<UVoxelSubstanceDataAsset>(VoxelData))
        {
            return SubstanceData->Material;
        }
    }
    return nullptr;
}

bool AVoxelWorld::IsVoxelCoordValid(const FIntVector& VoxelCoord) const
{
    return VoxelCoord.X >= 0 && VoxelCoord.X < WorldSize.X &&
        VoxelCoord.Y >= 0 && VoxelCoord.Y < WorldSize.Y &&
        VoxelCoord.Z >= 0 && VoxelCoord.Z < WorldSize.Z;
}

int32 AVoxelWorld::GetIndex(const FIntVector& VoxelCoord) const
{
    if (!IsVoxelCoordValid(VoxelCoord))
    {
        return -1;
    }
    return VoxelCoord.X + (VoxelCoord.Y * WorldSize.X) + (VoxelCoord.Z * WorldSize.X * WorldSize.Y);
}

int32 AVoxelWorld::GetVoxelID(const FIntVector& VoxelCoord) const
{
    int32 Index = GetIndex(VoxelCoord);
    if (Index != -1 && VoxelIDs.IsValidIndex(Index))
    {
        return VoxelIDs[Index];
    }
    return VoidID;
}

void AVoxelWorld::SetVoxelID(const FIntVector& VoxelCoord, int32 NewVoxelID)
{
    int32 Index = GetIndex(VoxelCoord);
    if (Index != -1 && VoxelIDs.IsValidIndex(Index))
    {
        VoxelIDs[Index] = NewVoxelID;
    }
}

float AVoxelWorld::GetDurability(const FIntVector& VoxelCoord) const
{
    int32 Index = GetIndex(VoxelCoord);
    if (Index != -1 && Durabilities.IsValidIndex(Index))
    {
        return Durabilities[Index];
    }
    return 0.0f;
}

void AVoxelWorld::SetDurability(const FIntVector& VoxelCoord, float NewDurability)
{
    int32 Index = GetIndex(VoxelCoord);
    if (Index != -1 && Durabilities.IsValidIndex(Index))
    {
        Durabilities[Index] = NewDurability;
    }
}

float AVoxelWorld::CalculateDensity(const FIntVector& VoxelCoord) const
{
    const UVoxelBaseDataAsset* VoxelData = GetVoxelData(GetVoxelID(VoxelCoord));

    if (VoxelData)
    {
        const float BaseDensity = VoxelData->BaseDensity;
        if (const UVoxelBlockDataAsset* BlockData = Cast<UVoxelBlockDataAsset>(VoxelData))
        {
            const float CurrentDurability = GetDurability(VoxelCoord);
            if (BlockData->MaxDurability > 0.f)
            {
                const float DurabilityRatio = FMath::Clamp(CurrentDurability / BlockData->MaxDurability, 0.f, 1.f);
                return FMath::Lerp(-1.0f, BaseDensity, DurabilityRatio);
            }
            return BaseDensity;
        }
        else
        {
            return BaseDensity;
        }
    }
    return 0.0f;
}
