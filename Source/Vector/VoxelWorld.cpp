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

    WorldVoxelDimensions = WorldSizeInChunks * CHUNK_SIZE;
    WorldVoxelVolume = WorldVoxelDimensions.X * WorldVoxelDimensions.Y * WorldVoxelDimensions.Z;

    GlobalVoxelIDMap.SetNumZeroed(WorldVoxelVolume);
    GlobalDurabilityMap.SetNumZeroed(WorldVoxelVolume);

    for (int32 i = 0; i < WorldVoxelVolume; ++i)
    {
        GlobalVoxelIDMap[i] = DefaultID;
        GlobalDurabilityMap[i] = 100;
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

    const FVector WorldMaxBounds = FVector(WorldVoxelDimensions) * VOXEL_SIZE;
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

        for (int32 z = FMath::Max(0, MinAffectedGrid.Z); z < FMath::Min(WorldVoxelDimensions.Z, MaxAffectedGrid.Z + 1); ++z)
        {
            for (int32 y = FMath::Max(0, MinAffectedGrid.Y); y < FMath::Min(WorldVoxelDimensions.Y, MaxAffectedGrid.Y + 1); ++y)
            {
                for (int32 x = FMath::Max(0, MinAffectedGrid.X); x < FMath::Min(WorldVoxelDimensions.X, MaxAffectedGrid.X + 1); ++x)
                {
                    FIntVector GlobalBlockCoord(x, y, z);
                    FVector WorldPosOfBlock = FVector(GlobalBlockCoord) * VOXEL_SIZE;

                    if (FVector::DistSquared(WorldPosOfBlock, NewRoom.Center) < FMath::Square(NewRoom.Radius))
                    {
                        SetGlobalVoxelID(GlobalBlockCoord, VoidID);
                        SetGlobalDurability(GlobalBlockCoord, 0.0f);
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

            NewChunk->DrawDebugBounds(-1.f, 5.f, FColor::Green);
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

bool AVoxelWorld::IsGlobalCoordValid(const FIntVector& GlobalCoord) const
{
    return GlobalCoord.X >= 0 && GlobalCoord.X < WorldVoxelDimensions.X &&
        GlobalCoord.Y >= 0 && GlobalCoord.Y < WorldVoxelDimensions.Y &&
        GlobalCoord.Z >= 0 && GlobalCoord.Z < WorldVoxelDimensions.Z;
}

int32 AVoxelWorld::GetGlobalIndex(const FIntVector& GlobalCoord) const
{
    if (!IsGlobalCoordValid(GlobalCoord))
    {
        return -1;
    }
    return GlobalCoord.X + (GlobalCoord.Y * WorldVoxelDimensions.X) + (GlobalCoord.Z * WorldVoxelDimensions.X * WorldVoxelDimensions.Y);
}

int32 AVoxelWorld::GetGlobalVoxelID(const FIntVector& GlobalCoord) const
{
    int32 Index = GetGlobalIndex(GlobalCoord);
    if (Index != -1 && GlobalVoxelIDMap.IsValidIndex(Index))
    {
        return GlobalVoxelIDMap[Index];
    }
    return VoidID;
}

void AVoxelWorld::SetGlobalVoxelID(const FIntVector& GlobalCoord, int32 NewVoxelID)
{
    int32 Index = GetGlobalIndex(GlobalCoord);
    if (Index != -1 && GlobalVoxelIDMap.IsValidIndex(Index))
    {
        UE_LOG(LogTemp, Log, TEXT("Set %d From %d to %d"), Index, GlobalVoxelIDMap[Index], NewVoxelID);
        GlobalVoxelIDMap[Index] = NewVoxelID;
    }
    else
        UE_LOG(LogTemp, Warning, TEXT("Invalid Coord"));
}

float AVoxelWorld::GetGlobalDurability(const FIntVector& GlobalCoord) const
{
    int32 Index = GetGlobalIndex(GlobalCoord);
    if (Index != -1 && GlobalDurabilityMap.IsValidIndex(Index))
    {
        return GlobalDurabilityMap[Index];
    }
    UE_LOG(LogTemp, Warning, TEXT("Invalid Coord"));
    return 0.0f;
}

void AVoxelWorld::SetGlobalDurability(const FIntVector& GlobalCoord, float NewDurability)
{
    int32 Index = GetGlobalIndex(GlobalCoord);
    if (Index != -1 && GlobalDurabilityMap.IsValidIndex(Index))
    {
        GlobalDurabilityMap[Index] = NewDurability;
    }
    else
        UE_LOG(LogTemp, Warning, TEXT("Invalid Coord"));
}

float AVoxelWorld::CalculateGlobalDensity(const FIntVector& GlobalCoord) const
{
    const UVoxelBaseDataAsset* VoxelData = GetVoxelData(GetGlobalVoxelID(GlobalCoord));

    if (VoxelData)
    {
        const float BaseDensity = VoxelData->BaseDensity;
        if (const UVoxelBlockDataAsset* BlockData = Cast<UVoxelBlockDataAsset>(VoxelData))
        {
            const float CurrentDurability = GetGlobalDurability(GlobalCoord);
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
    UE_LOG(LogTemp, Warning, TEXT("Invalid Coord"));
    return -1.0f;
}

FVector AVoxelWorld::GetSmoothNormalAtGlobalCoord(const FIntVector& GlobalCoord) const
{
    float DensityXPlus = CalculateGlobalDensity(GlobalCoord + FIntVector(1, 0, 0));
    float DensityXMinus = CalculateGlobalDensity(GlobalCoord - FIntVector(1, 0, 0));
    float DensityYPlus = CalculateGlobalDensity(GlobalCoord + FIntVector(0, 1, 0));
    float DensityYMinus = CalculateGlobalDensity(GlobalCoord - FIntVector(0, 1, 0));
    float DensityZPlus = CalculateGlobalDensity(GlobalCoord + FIntVector(0, 0, 1));
    float DensityZMinus = CalculateGlobalDensity(GlobalCoord - FIntVector(0, 0, 1));

    FVector Normal = FVector(
        DensityXPlus - DensityXMinus,
        DensityYPlus - DensityYMinus,
        DensityZPlus - DensityZMinus
    );

    return Normal.GetSafeNormal();
}