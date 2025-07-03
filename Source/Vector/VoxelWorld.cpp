#include "VoxelWorld.h"
#include "VoxelChunk.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

AVoxelWorld::AVoxelWorld()
{
    PrimaryActorTick.bCanEverTick = false;

    WorldSize = WorldSizeInChunks * CHUNK_SIZE;
    WorldVolume = WorldSize.X * WorldSize.Y * WorldSize.Z;
}

void AVoxelWorld::Initialize(int32 NumberOfPlayers)
{
    VoxelDataMap.Empty();
    VoxelDataMap.Add(GetVoidID(), VoxelVoidDataAsset);
    VoxelDataMap.Add(GetBorderID(), VoxelBorderDataAsset);
    for (int32 i = 0; i < VoxelBlockDataAssets.Num(); ++i)
        if (VoxelBlockDataAssets[i]) VoxelDataMap.Add(i + GetDefaultBlockID(), VoxelBlockDataAssets[i]);

    VoxelIDs.SetNumZeroed(WorldVolume);
    Durabilities.SetNumZeroed(WorldVolume);

    float DefaultBlockMaxDurability;
    UVoxelBlockDataAsset * DefaultBlockData = Cast<UVoxelBlockDataAsset>(GetVoxelData(GetDefaultBlockID()));
    if (DefaultBlockData)
        DefaultBlockMaxDurability = DefaultBlockData->MaxDurability;
    else
        DefaultBlockMaxDurability = 100.f;


    for (int32 x = 0; x < WorldSize.X; ++x)
    {
        for (int32 y = 0; y < WorldSize.Y; ++y)
        {
            for (int32 z = 0; z < WorldSize.Z; ++z)
            {
                const FIntVector VoxelCoord = FIntVector(x, y, z);

                if (!x || !y || !z || x == WorldSize.X - 1 || y == WorldSize.Y - 1 || z == WorldSize.Z - 1)
                    VoxelIDs[GetIndex(VoxelCoord)] = GetBorderID();
                else {
                    VoxelIDs[GetIndex(VoxelCoord)] = GetDefaultBlockID();
                    Durabilities[GetIndex(VoxelCoord)] = DefaultBlockMaxDurability;
                }
            }
        }
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
        if (StartPoint) StartPoint->Destroy();
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
                        SetVoxelID(GlobalBlockCoord, GetVoidID());
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
        NewChunk->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
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
    return VoxelDataMap.FindRef(GetVoidID());
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
    return VoxelCoord.X + (VoxelCoord.Y * WorldSize.X) + (VoxelCoord.Z * WorldSize.X * WorldSize.Y);
}

int32 AVoxelWorld::GetVoxelID(const FIntVector& VoxelCoord) const
{
    if (!IsVoxelCoordValid(VoxelCoord)) return GetVoidID();

    const int32 Index = GetIndex(VoxelCoord);

    if (VoxelIDs.IsValidIndex(Index))
        return VoxelIDs[Index];

    return GetVoidID();
}

void AVoxelWorld::SetVoxelID(const FIntVector& VoxelCoord, int32 NewVoxelID)
{
    if (!IsVoxelCoordValid(VoxelCoord)) return;

    const int32 Index = GetIndex(VoxelCoord);

    if (VoxelIDs.IsValidIndex(Index))
        VoxelIDs[Index] = NewVoxelID;
}

float AVoxelWorld::GetDurability(const FIntVector& VoxelCoord) const
{
    if (!IsVoxelCoordValid(VoxelCoord)) return 0.f;

    const int32 Index = GetIndex(VoxelCoord);

    if (Durabilities.IsValidIndex(Index))
        return Durabilities[Index];

    return 0.f;
}

void AVoxelWorld::SetDurability(const FIntVector& VoxelCoord, float NewDurability)
{
    if (!IsVoxelCoordValid(VoxelCoord)) return;

    const int32 Index = GetIndex(VoxelCoord);
    if (Durabilities.IsValidIndex(Index))
        Durabilities[Index] = NewDurability;
}

float AVoxelWorld::CalculateDensity(const FIntVector& VoxelCoord) const
{
    const UVoxelBaseDataAsset* VoxelData = GetVoxelData(GetVoxelID(VoxelCoord));

    if (VoxelData)
    {
        const float BaseDensity = VoxelData->BaseDensity;
        if (const UVoxelBlockDataAsset* BlockData = Cast<UVoxelBlockDataAsset>(VoxelData))
        {
            const float DurabilityRatio = FMath::Clamp(GetDurability(VoxelCoord) / BlockData->MaxDurability, 0.f, 1.f);
            return FMath::Lerp(-1.0f, BaseDensity, DurabilityRatio);
        }
        else return BaseDensity;
    }

    return 0.f;
}


void AVoxelWorld::DamageVoxel(const FVector& Center, float Radius, float DamageAmount)
{
    TArray<FIntVector> FoundVoxels;

    const FIntVector CenterVoxelCoord = FIntVector(
        FMath::RoundToInt(Center.X / VOXEL_SIZE),
        FMath::RoundToInt(Center.Y / VOXEL_SIZE),
        FMath::RoundToInt(Center.Z / VOXEL_SIZE)
    );
    const int32 RadiusInVoxels = FMath::CeilToInt(Radius / VOXEL_SIZE);
    const float RadiusSquared = FMath::Square(Radius);

    for (int32 z = CenterVoxelCoord.Z - RadiusInVoxels; z <= CenterVoxelCoord.Z + RadiusInVoxels; ++z)
    {
        for (int32 y = CenterVoxelCoord.Y - RadiusInVoxels; y <= CenterVoxelCoord.Y + RadiusInVoxels; ++y)
        {
            for (int32 x = CenterVoxelCoord.X - RadiusInVoxels; x <= CenterVoxelCoord.X + RadiusInVoxels; ++x)
            {
                const FIntVector VoxelCoord(x, y, z);
                if (!IsVoxelCoordValid(VoxelCoord)) continue;

                const FVector VoxelWorldPos = (FVector(VoxelCoord) * VOXEL_SIZE) + (VOXEL_SIZE / 2.0f);
                if (FVector::DistSquared(VoxelWorldPos, Center) < RadiusSquared)
                {
                    FoundVoxels.Add(VoxelCoord);
                }
            }
        }
    }

    TArray<FIntVector> SurfaceVoxels;
    const TArray<FIntVector> NeighborOffsets = {
        FIntVector(1, 0, 0), FIntVector(-1, 0, 0), FIntVector(0, 1, 0),
        FIntVector(0, -1, 0), FIntVector(0, 0, 1), FIntVector(0, 0, -1)
    };

    for (const FIntVector& VoxelCoord : FoundVoxels)
    {
        const UVoxelBaseDataAsset* VoxelData = GetVoxelData(GetVoxelID(VoxelCoord));
        const UVoxelBlockDataAsset* BlockData = Cast<UVoxelBlockDataAsset>(VoxelData);

        if (!BlockData)
        {
            continue;
        }

        if (GetVoxelID(VoxelCoord) == GetVoidID())
        {
            continue;
        }

        bool bIsSurface = false;
        for (const FIntVector& Offset : NeighborOffsets)
        {
            if (!IsVoxelCoordValid(VoxelCoord + Offset) || GetVoxelID(VoxelCoord + Offset) == GetVoidID())
            {
                bIsSurface = true;
                break;
            }
        }

        if (bIsSurface)
        {
            SurfaceVoxels.Add(VoxelCoord);
        }
    }

    if (SurfaceVoxels.Num() == 0)
    {
        return;
    }

    TSet<FIntVector> DirtyChunkCoords;

    for (const FIntVector& VoxelCoord : SurfaceVoxels)
    {
        const float NewDurability = GetDurability(VoxelCoord) - DamageAmount;

        if (NewDurability <= 0)
        {
            SetVoxelID(VoxelCoord, GetVoidID());
            SetDurability(VoxelCoord, 0.f);
        }
        else
        {
            SetDurability(VoxelCoord, NewDurability);
        }

        const FIntVector ChunkCoord = FIntVector(
            FMath::FloorToInt(static_cast<float>(VoxelCoord.X) / CHUNK_SIZE),
            FMath::FloorToInt(static_cast<float>(VoxelCoord.Y) / CHUNK_SIZE),
            FMath::FloorToInt(static_cast<float>(VoxelCoord.Z) / CHUNK_SIZE)
        );
        DirtyChunkCoords.Add(ChunkCoord);

        const int32 LastVoxelIndexInChunk = CHUNK_SIZE - 1;
        const int32 LocalX = VoxelCoord.X % CHUNK_SIZE;
        const int32 LocalY = VoxelCoord.Y % CHUNK_SIZE;
        const int32 LocalZ = VoxelCoord.Z % CHUNK_SIZE;

        if (LocalX == 0)                 DirtyChunkCoords.Add(ChunkCoord + FIntVector(-1, 0, 0));
        if (LocalX == LastVoxelIndexInChunk) DirtyChunkCoords.Add(ChunkCoord + FIntVector(1, 0, 0));
        if (LocalY == 0)                 DirtyChunkCoords.Add(ChunkCoord + FIntVector(0, -1, 0));
        if (LocalY == LastVoxelIndexInChunk) DirtyChunkCoords.Add(ChunkCoord + FIntVector(0, 1, 0));
        if (LocalZ == 0)                 DirtyChunkCoords.Add(ChunkCoord + FIntVector(0, 0, -1));
        if (LocalZ == LastVoxelIndexInChunk) DirtyChunkCoords.Add(ChunkCoord + FIntVector(0, 0, 1));
    }

    for (const FIntVector& CoordToUpdate : DirtyChunkCoords)
    {
        if (CoordToUpdate.X >= 0 && CoordToUpdate.X < WorldSizeInChunks.X &&
            CoordToUpdate.Y >= 0 && CoordToUpdate.Y < WorldSizeInChunks.Y &&
            CoordToUpdate.Z >= 0 && CoordToUpdate.Z < WorldSizeInChunks.Z)
        {
            if (TObjectPtr<AVoxelChunk>* FoundChunk = Chunks.Find(CoordToUpdate))
            {
                if (*FoundChunk)
                {
                    (*FoundChunk)->GenerateMesh();
                }
            }
        }
    }
}