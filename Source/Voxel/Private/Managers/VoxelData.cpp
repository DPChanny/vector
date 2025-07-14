#include "Managers/VoxelData.h"

#include "Actors/VoxelChunkActor.h"
#include "Actors/VoxelWorld.h"
#include "DataAssets/VoxelBaseDataAsset.h"
#include "DataAssets/VoxelBlockDataAsset.h"
#include "DataAssets/VoxelBorderDataAsset.h"
#include "DataAssets/VoxelVoidDataAsset.h"
#include "Managers/VoxelDebug.h"
#include "Managers/VoxelMesh.h"

int32 UVoxelData::GetVoxelID(
    const TObjectPtr<UVoxelBaseDataAsset> &VoxelDataAsset) {
  for (const auto &Elem : VoxelDataAssets) {
    if (Elem.Value == VoxelDataAsset) {
      return Elem.Key;
    }
  }
  return GetVoidID();
}

void UVoxelData::Initialize(
    const FIntVector &InWorldSizeInChunks, const int32 InChunkSize,
    const int32 InVoxelSize,
    const TSubclassOf<AVoxelChunkActor> &InVoxelChunkActor,
    const TArray<TObjectPtr<UVoxelBlockDataAsset>> &InVoxelBlockDataAssets,
    const TObjectPtr<UVoxelVoidDataAsset> &InVoxelVoidDataAsset,
    const TObjectPtr<UVoxelBorderDataAsset> &InVoxelBorderDataAsset) {
  const AVoxelWorld *VoxelWorld = Cast<AVoxelWorld>(GetOuter());
  VoxelDebug = VoxelWorld->GetVoxelDebug();
  VoxelMesh = VoxelWorld->GetVoxelMesh();
  WorldSizeInChunks = InWorldSizeInChunks;
  ChunkSize = InChunkSize;
  VoxelSize = InVoxelSize;
  ChunkVolume = ChunkSize * ChunkSize * ChunkSize;
  VoxelChunk = InVoxelChunkActor;

  VoxelDataAssets.Empty();
  if (const TObjectPtr<UVoxelBaseDataAsset> VoidDataAsset =
          Cast<UVoxelBaseDataAsset>(InVoxelVoidDataAsset)) {
    VoxelDataAssets.Add(GetVoidID(), VoidDataAsset);
  }
  if (const TObjectPtr<UVoxelBaseDataAsset> BorderDataAsset =
          Cast<UVoxelBaseDataAsset>(InVoxelBorderDataAsset)) {
    VoxelDataAssets.Add(GetBorderID(), BorderDataAsset);
  }
  for (int32 i = 0; i < InVoxelBlockDataAssets.Num(); ++i) {
    if (InVoxelBlockDataAssets[i]) {
      VoxelDataAssets.Add(i + GetDefaultBlockID(), InVoxelBlockDataAssets[i]);
    }
  }

  Chunks.Empty();
}

void UVoxelData::LoadChunk(const FIntVector &ChunkCoord) {
  if (Chunks.Contains(ChunkCoord)) {
    return;
  }

  if (!VoxelChunk) {
    return;
  }

  FVoxelChunk NewChunk;
  NewChunk.Initialize(ChunkVolume);

  UWorld *World = GetWorld();
  if (!World) {
    return;
  }

  FActorSpawnParameters SpawnParams;
  SpawnParams.Owner = Cast<AActor>(GetOuter());
  const FVector ChunkWorldLocation =
      FVector(ChunkCoord) * ChunkSize * VoxelSize;

  AVoxelChunkActor *NewVoxelChunk = World->SpawnActor<AVoxelChunkActor>(
      VoxelChunk, ChunkWorldLocation, FRotator::ZeroRotator, SpawnParams);

  if (NewVoxelChunk) {
    if (AActor *Owner = Cast<AActor>(GetOuter())) {
      NewVoxelChunk->AttachToActor(
          Owner, FAttachmentTransformRules::KeepWorldTransform);
    }
    NewVoxelChunk->Initialize(ChunkCoord);
    NewChunk.VoxelChunkActor = NewVoxelChunk;

    Chunks.Add(ChunkCoord, MoveTemp(NewChunk));
  }
}

void UVoxelData::UnloadChunk(const FIntVector &ChunkCoord) {
  if (const FVoxelChunk *Chunk = Chunks.Find(ChunkCoord)) {
    if (Chunk->VoxelChunkActor) {
      Chunk->VoxelChunkActor->Destroy();
    }
    Chunks.Remove(ChunkCoord);
  }
}

FVoxel UVoxelData::GetVoxel(const FIntVector &GlobalCoord) const {
  const FIntVector ChunkCoord = GlobalToChunkCoord(GlobalCoord);
  if (const FVoxelChunk *Chunk = Chunks.Find(ChunkCoord)) {
    const FIntVector LocalCoord = GlobalToLocalCoord(GlobalCoord);
    const int32 Index = LocalCoordToIndex(LocalCoord);
    return Chunk->GetVoxel(Index);
  }
  return FVoxel(GetVoidID(), 0.f);
}

void UVoxelData::SetVoxel(const FIntVector &GlobalCoord, const FVoxel &Voxel,
                          const bool bAutoDebug) {
  const FIntVector ChunkCoord = GlobalToChunkCoord(GlobalCoord);
  if (FVoxelChunk *Chunk = Chunks.Find(ChunkCoord)) {
    const FIntVector LocalCoord = GlobalToLocalCoord(GlobalCoord);
    const int32 Index = LocalCoordToIndex(LocalCoord);
    Chunk->SetVoxel(Index, Voxel);

    if (VoxelDebug && bAutoDebug) {
      VoxelDebug->SetDebugVoxel(GlobalCoord);
    }

    if (VoxelMesh) {
      VoxelMesh->SetDirtyChunk(GlobalCoord);
    }
  }
}

int32 UVoxelData::GetVoxelID(const FIntVector &GlobalCoord) const {
  return GetVoxel(GlobalCoord).ID;
}

void UVoxelData::SetVoxelID(const FIntVector &GlobalCoord,
                            const int32 NewVoxelID) {
  FVoxel CurrentVoxel = GetVoxel(GlobalCoord);
  CurrentVoxel.ID = NewVoxelID;
  SetVoxel(GlobalCoord, CurrentVoxel);
}

float UVoxelData::GetDurability(const FIntVector &GlobalCoord) const {
  return GetVoxel(GlobalCoord).Durability;
}

void UVoxelData::SetDurability(const FIntVector &GlobalVoxelCoord,
                               const float NewDurability) {
  FVoxel CurrentVoxel = GetVoxel(GlobalVoxelCoord);
  CurrentVoxel.Durability = NewDurability;
  SetVoxel(GlobalVoxelCoord, CurrentVoxel);
}

float UVoxelData::GetDensity(const FIntVector &GlobalCoord) const {
  const FVoxel Voxel = GetVoxel(GlobalCoord);
  if (const UVoxelBaseDataAsset *VoxelAsset =
          GetVoxelDataAsset<UVoxelBaseDataAsset>(Voxel.ID)) {
    const float BaseDensity = VoxelAsset->BaseDensity;
    if (const UVoxelBlockDataAsset *BlockData =
            Cast<UVoxelBlockDataAsset>(VoxelAsset)) {
      return BaseDensity * Voxel.Durability / BlockData->MaxDurability;
    }
    return BaseDensity;
  }
  return -1.f;
}

bool UVoxelData::IsChunk(const FIntVector &ChunkCoord) const {
  return Chunks.Contains(ChunkCoord);
}

FVoxelChunk *UVoxelData::GetChunk(const FIntVector &ChunkCoord) {
  return Chunks.Find(ChunkCoord);
}

FIntVector UVoxelData::GlobalToChunkCoord(const FIntVector &GlobalCoord) const {
  return FIntVector(
      FMath::FloorToInt(static_cast<float>(GlobalCoord.X) / ChunkSize),
      FMath::FloorToInt(static_cast<float>(GlobalCoord.Y) / ChunkSize),
      FMath::FloorToInt(static_cast<float>(GlobalCoord.Z) / ChunkSize));
}

FIntVector UVoxelData::ChunkToGlobalCoord(const FIntVector &ChunkCoord) const {
  return ChunkCoord * ChunkSize;
}

FIntVector UVoxelData::GlobalToLocalCoord(const FIntVector &GlobalCoord) const {
  return GlobalCoord % ChunkSize;
}

FIntVector UVoxelData::LocalToGlobalCoord(const FIntVector &LocalCoord,
                                          const FIntVector &ChunkCoord) const {
  return ChunkToGlobalCoord(ChunkCoord) + LocalCoord;
}

FIntVector UVoxelData::WorldToGlobalCoord(const FVector &WorldCoord) const {
  return FIntVector(FMath::RoundToInt(WorldCoord.X / VoxelSize),
                    FMath::RoundToInt(WorldCoord.Y / VoxelSize),
                    FMath::RoundToInt(WorldCoord.Z / VoxelSize));
}

FVector UVoxelData::GlobalToWorldCoord(const FIntVector &GlobalCoord) const {
  return FVector(GlobalCoord) * VoxelSize;
}

int32 UVoxelData::LocalCoordToIndex(const FIntVector &LocalCoord) const {
  return LocalCoord.X + LocalCoord.Y * ChunkSize +
         LocalCoord.Z * ChunkSize * ChunkSize;
}