#include "VoxelWorld.h"
#include "VoxelChunk.h"
#include "VoxelBlockDataAsset.h"
#include "VoxelVoidDataAsset.h"
#include "VoxelBorderDataAsset.h"
#include "VoxelDebugActor.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

AVoxelWorld::AVoxelWorld()
{
    PrimaryActorTick.bCanEverTick = false;

    WorldSize = WorldSizeInChunks * ChunkSize;
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

    InitializeNexuses(NumberOfPlayers);

    for (int32 x = 0; x < WorldSizeInChunks.X; ++x)
        for (int32 y = 0; y < WorldSizeInChunks.Y; ++y)
            for (int32 z = 0; z < WorldSizeInChunks.Z; ++z)
                InitializeChunk(FIntVector(x, y, z));
}

void AVoxelWorld::InitializeNexuses(int32 NexusCount)
{
    Nexuses.Empty();
    for (APlayerStart* StartPoint : PlayerStarts)
        if (StartPoint) StartPoint->Destroy();
    PlayerStarts.Empty();

    const FVector WorldMaxBounds = FVector(WorldSize) * VoxelSize;
    const FVector WorldMinBounds = FVector::ZeroVector;

    const int32 MaxPlacementAttempts = 100;

    for (int32 i = 0; i < NexusCount; ++i)
    {
        FNexus NewRoom;
        NewRoom.Radius = NexusRadius;

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
            for (const FNexus& ExistingRoom : Nexuses)
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
                Nexuses.Add(NewRoom);
                bRoomPlaced = true;
                break;
            }
        }

        if (!bRoomPlaced)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to place room %d after %d attempts. Skipping."), i, MaxPlacementAttempts);
            continue;
        }

        FIntVector MinAffectedGrid = FIntVector(FVector(NewRoom.Center - NewRoom.Radius) / VoxelSize);
        FIntVector MaxAffectedGrid = FIntVector(FVector(NewRoom.Center + NewRoom.Radius) / VoxelSize);

        for (int32 z = FMath::Max(0, MinAffectedGrid.Z); z < FMath::Min(WorldSize.Z, MaxAffectedGrid.Z + 1); ++z)
        {
            for (int32 y = FMath::Max(0, MinAffectedGrid.Y); y < FMath::Min(WorldSize.Y, MaxAffectedGrid.Y + 1); ++y)
            {
                for (int32 x = FMath::Max(0, MinAffectedGrid.X); x < FMath::Min(WorldSize.X, MaxAffectedGrid.X + 1); ++x)
                {
                    FIntVector GlobalBlockCoord(x, y, z);
                    FVector WorldPosOfBlock = FVector(GlobalBlockCoord) * VoxelSize;

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
            PlayerStarts.Add(NewPlayerStart);
            UE_LOG(LogTemp, Log, TEXT("Generated PlayerStart at: %s"), *NewPlayerStart->GetActorLocation().ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn PlayerStart for room %d at %s"), i, *NewRoom.Center.ToString());
        }
    }
    UE_LOG(LogTemp, Log, TEXT("VoxelWorld: Generated %d PlayerStarts."), PlayerStarts.Num());
}

void AVoxelWorld::InitializeChunk(const FIntVector& ChunkCoord)
{
    if (!Chunks.Contains(ChunkCoord))
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        FVector ChunkWorldLocation = FVector(ChunkCoord) * ChunkSize * VoxelSize;
        AVoxelChunk* NewChunk = GetWorld()->SpawnActor<AVoxelChunk>(ChunkWorldLocation, FRotator::ZeroRotator, SpawnParams);
        NewChunk->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
        if (NewChunk)
        {
            NewChunk->Initialize(ChunkCoord);
            Chunks.Add(ChunkCoord, NewChunk);
        }
    }
}

UVoxelBaseDataAsset* AVoxelWorld::GetVoxelData(int32 VoxelID) const
{
    if (const TObjectPtr<UVoxelBaseDataAsset> FoundData = VoxelDataMap.FindRef(VoxelID)) return FoundData;
    return VoxelDataMap.FindRef(GetVoidID());
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

const int32 AVoxelWorld::GetVoxelID(const TObjectPtr<UVoxelBaseDataAsset>& VoxelDataAsset)
{
    for (const auto& Pair : VoxelDataMap) {
        if (Pair.Value == VoxelDataAsset)
            return Pair.Key;
    }
    return GetVoidID();
}

const int32 AVoxelWorld::GetVoxelID(const FIntVector& VoxelCoord) const
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

    SetDebugVoxel(VoxelCoord);
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

    SetDebugVoxel(VoxelCoord);
}

float AVoxelWorld::GetDensity(const FIntVector& VoxelCoord) const
{
    const UVoxelBaseDataAsset* VoxelData = GetVoxelData(GetVoxelID(VoxelCoord));

    if (VoxelData)
    {
        const float BaseDensity = VoxelData->BaseDensity;
        if (const UVoxelBlockDataAsset* BlockData = Cast<UVoxelBlockDataAsset>(VoxelData))
            return BaseDensity * GetDurability(VoxelCoord) / BlockData->MaxDurability;
        else return BaseDensity;
    }

    return -1.f;
}


void AVoxelWorld::DamageVoxel(const FVector& Center, float Radius, float DamageAmount)
{
    auto DamageLogic = [&](const FIntVector& VoxelCoord, TSet<FIntVector>& DirtyChunks)
        {
            if (!Cast<UVoxelBlockDataAsset>(GetVoxelData(GetVoxelID(VoxelCoord)))) return;

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

            AddDirtyChunk(VoxelCoord, DirtyChunks);
        };

    ProcessVoxel(Center, Radius, DamageLogic);
}



void AVoxelWorld::ConstructVoxel(const FVector& Center, float Radius, float ConstructionAmount, int32 NewVoxelID)
{
    auto ConstructLogic = [&](const FIntVector& VoxelCoord, TSet<FIntVector>& DirtyChunks)
        {
            const UVoxelBlockDataAsset* VoxelData = Cast<UVoxelBlockDataAsset>(GetVoxelData(GetVoxelID(VoxelCoord)));
            if (!VoxelData) return;

            const UVoxelBlockDataAsset* NewVoxelData = Cast<UVoxelBlockDataAsset>(GetVoxelData(NewVoxelID));
            if (!NewVoxelData) return;

            const float NewDurability = GetDurability(VoxelCoord) + ConstructionAmount;

            if (NewDurability >= VoxelData->MaxDurability)
            {
                SetDurability(VoxelCoord, VoxelData->MaxDurability);

                const TArray<FIntVector> NeighborOffsets = {
                    FIntVector(1, 0, 0), FIntVector(-1, 0, 0), FIntVector(0, 1, 0),
                    FIntVector(0, -1, 0), FIntVector(0, 0, 1), FIntVector(0, 0, -1)
                };

                for (const FIntVector& NeighborOffset : NeighborOffsets)
                {
                    const FIntVector NeighborCoord = VoxelCoord + NeighborOffset;
                    if (IsVoxelCoordValid(NeighborCoord) && GetVoxelID(NeighborCoord) == GetVoidID())
                    {
                        SetVoxelID(NeighborCoord, NewVoxelID);

                        for (const FIntVector& CheckOffset : NeighborOffsets)
                        {
                            const FIntVector CheckCoord = NeighborCoord + CheckOffset;

                            if (CheckCoord == VoxelCoord) continue;

                            if (const UVoxelBlockDataAsset* EnclosedBlockData = Cast<UVoxelBlockDataAsset>(GetVoxelData(GetVoxelID(CheckCoord))))
                            {
                                if (!IsSurfaceVoxel(CheckCoord))
                                {
                                    SetDurability(CheckCoord, EnclosedBlockData->MaxDurability);
                                    AddDirtyChunk(CheckCoord, DirtyChunks);
                                }
                            }
                        }

                        if (!IsSurfaceVoxel(NeighborCoord))
                            SetDurability(NeighborCoord, NewVoxelData->MaxDurability);

                        AddDirtyChunk(NeighborCoord, DirtyChunks);
                    }
                }
            }
            else
            {
                SetDurability(VoxelCoord, NewDurability);
            }

            AddDirtyChunk(VoxelCoord, DirtyChunks);
        };

    ProcessVoxel(Center, Radius, ConstructLogic);
}

FIntVector AVoxelWorld::WorldPosToVoxelCoord(const FVector& WorldPos) const
{
    return FIntVector(
        FMath::RoundToInt(WorldPos.X / VoxelSize), 
        FMath::RoundToInt(WorldPos.Y / VoxelSize),
        FMath::RoundToInt(WorldPos.Z / VoxelSize));
}

FIntVector AVoxelWorld::VoxelCoordToChunkCoord(const FIntVector& VoxelCoord)
{
    return FIntVector(
        FMath::FloorToInt(static_cast<float>(VoxelCoord.X) / ChunkSize),
        FMath::FloorToInt(static_cast<float>(VoxelCoord.Y) / ChunkSize),
        FMath::FloorToInt(static_cast<float>(VoxelCoord.Z) / ChunkSize)
    );
}


void AVoxelWorld::GetVoxelCoordsInRadius(const FVector& Center, float Radius, TSet<FIntVector>& FoundVoxelCoords)
{
    const FIntVector CenterVoxelCoord = WorldPosToVoxelCoord(Center);
    const int32 RadiusInVoxels = FMath::CeilToInt(Radius / VoxelSize);
    const float RadiusSquared = FMath::Square(Radius);

    for (int32 z = CenterVoxelCoord.Z - RadiusInVoxels; z <= CenterVoxelCoord.Z + RadiusInVoxels; ++z)
    {
        for (int32 y = CenterVoxelCoord.Y - RadiusInVoxels; y <= CenterVoxelCoord.Y + RadiusInVoxels; ++y)
        {
            for (int32 x = CenterVoxelCoord.X - RadiusInVoxels; x <= CenterVoxelCoord.X + RadiusInVoxels; ++x)
            {
                const FIntVector VoxelCoord(x, y, z);

                if (!IsVoxelCoordValid(VoxelCoord))
                {
                    continue;
                }

                const FVector VoxelWorldPos = FVector(VoxelCoord) * VoxelSize + (FVector(VoxelSize) * 0.5f);
                if (FVector::DistSquared(VoxelWorldPos, Center) >= RadiusSquared)
                {
                    continue;
                }

                FoundVoxelCoords.Add(VoxelCoord);
            }
        }
    }
}

void AVoxelWorld::ProcessVoxel(const FVector& Center, float Radius, TFunction<void(const FIntVector&, TSet<FIntVector>&)> VoxelModifier)
{
    TSet<FIntVector> VoxelCoordsInRadius;
    TSet<FIntVector> VoxelCoordsToProcess;
    GetVoxelCoordsInRadius(Center, Radius, VoxelCoordsInRadius);

    TSet<FIntVector> DirtyChunkCoords;

    for (const FIntVector& VoxelCoord : VoxelCoordsInRadius)
    {
        if (GetVoxelID(VoxelCoord) == GetVoidID()) continue;

        if (IsSurfaceVoxel(VoxelCoord))
			VoxelCoordsToProcess.Add(VoxelCoord);
    }

	for (const FIntVector& VoxelCoord : VoxelCoordsToProcess)
        VoxelModifier(VoxelCoord, DirtyChunkCoords);

    UpdateDirtyChunk(DirtyChunkCoords);
}

void AVoxelWorld::AddDirtyChunk(const FIntVector& VoxelCoord, TSet<FIntVector>& DirtyChunks)
{
    const FIntVector ChunkCoord = VoxelCoordToChunkCoord(VoxelCoord);

    const int32 LocalX = VoxelCoord.X % ChunkSize;
    const int32 LocalY = VoxelCoord.Y % ChunkSize;
    const int32 LocalZ = VoxelCoord.Z % ChunkSize;

    const int32 LastVoxelIndexInChunk = ChunkSize - 1;

    const int32 StartX = (LocalX == 0) ? -1 : 0;
    const int32 EndX = (LocalX == LastVoxelIndexInChunk) ? 1 : 0;

    const int32 StartY = (LocalY == 0) ? -1 : 0;
    const int32 EndY = (LocalY == LastVoxelIndexInChunk) ? 1 : 0;

    const int32 StartZ = (LocalZ == 0) ? -1 : 0;
    const int32 EndZ = (LocalZ == LastVoxelIndexInChunk) ? 1 : 0;

    for (int32 i = StartX; i <= EndX; ++i)
        for (int32 j = StartY; j <= EndY; ++j)
            for (int32 k = StartZ; k <= EndZ; ++k)
                DirtyChunks.Add(ChunkCoord + FIntVector(i, j, k));
}

void AVoxelWorld::UpdateDirtyChunk(const TSet<FIntVector>& DirtyChunks)
{
    for (const FIntVector& CoordToUpdate : DirtyChunks)
    {
        if (CoordToUpdate.X >= 0 && CoordToUpdate.X < WorldSizeInChunks.X &&
            CoordToUpdate.Y >= 0 && CoordToUpdate.Y < WorldSizeInChunks.Y &&
            CoordToUpdate.Z >= 0 && CoordToUpdate.Z < WorldSizeInChunks.Z)
        {
            if (TObjectPtr<AVoxelChunk>* FoundChunk = Chunks.Find(CoordToUpdate))
            {
                if (*FoundChunk)
                {
                    (*FoundChunk)->UpdateMesh();
                }
            }
        }
    }
}

bool AVoxelWorld::IsSurfaceVoxel(const FIntVector& VoxelCoord) const
{
    const TArray<FIntVector> NeighborOffsets = {
        FIntVector(1, 0, 0), FIntVector(-1, 0, 0), FIntVector(0, 1, 0),
        FIntVector(0, -1, 0), FIntVector(0, 0, 1), FIntVector(0, 0, -1)
    };

    for (const FIntVector& Offset : NeighborOffsets)
        if (!IsVoxelCoordValid(VoxelCoord + Offset) || GetVoxelID(VoxelCoord + Offset) == GetVoidID())
            return true;

    return false;
}

void AVoxelWorld::SetDebugVoxel(const FIntVector& NewDebugVoxel)
{
    DebugVoxelsBuffer.Add(NewDebugVoxel);
}

void AVoxelWorld::SetDebugVoxels(const TSet<FIntVector>& NewDebugVoxels)
{
    DebugVoxelsBuffer.Append(NewDebugVoxels);
}

void AVoxelWorld::FlushDebugVoxelBuffer()
{
    TSet<FIntVector> CurrentDebugVoxels;
    DebugVoxels.GetKeys(CurrentDebugVoxels);

    const TSet<FIntVector> VoxelsToRemove = CurrentDebugVoxels.Difference(DebugVoxelsBuffer);
    for (const FIntVector& VoxelToRemove : VoxelsToRemove)
        RemoveDebugVoxel(VoxelToRemove);

    const TSet<FIntVector> VoxelsToUpdate = CurrentDebugVoxels.Difference(VoxelsToRemove);
    for (const FIntVector& VoxelToUpdate : VoxelsToUpdate)
        if (const TObjectPtr<AVoxelDebugActor>* FoundActor = DebugVoxels.Find(VoxelToUpdate))
            (*FoundActor)->UpdateWidget();

    const TSet<FIntVector> VoxelsToAdd = DebugVoxelsBuffer.Difference(CurrentDebugVoxels);
    for (const FIntVector& VoxelToAdd : VoxelsToAdd)
        AddDebugVoxel(VoxelToAdd);

    DebugVoxelsBuffer.Reset();
}

void AVoxelWorld::AddDebugVoxel(const FIntVector& VoxelCoord)
{
    if (DebugVoxels.Contains(VoxelCoord))
    {
        return;
    }

    if (!DebugActorClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("DebugActorClass is not set in VoxelWorld."));
        return;
    }

    const FVector SpawnLocation = (FVector(VoxelCoord) * VoxelSize);

    AVoxelDebugActor* NewDebugActor = GetWorld()->SpawnActor<AVoxelDebugActor>(DebugActorClass, SpawnLocation, FRotator::ZeroRotator);

    if (NewDebugActor)
    {
        NewDebugActor->Initialize(VoxelCoord);
        DebugVoxels.Add(VoxelCoord, NewDebugActor);
    }
}

void AVoxelWorld::RemoveDebugVoxel(const FIntVector& VoxelCoord)
{
    if (TObjectPtr<AVoxelDebugActor> FoundActor = DebugVoxels.FindRef(VoxelCoord))
    {
        if (FoundActor)
        {
            FoundActor->Destroy();
        }
        DebugVoxels.Remove(VoxelCoord);
    }
}
