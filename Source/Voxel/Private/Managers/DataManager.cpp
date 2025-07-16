#include "Managers/DataManager.h"

#include "Actors/VoxelChunkActor.h"
#include "Actors/VoxelWorld.h"
#include "DataAssets/VoxelBaseDataAsset.h"
#include "Managers/DebugManager.h"
#include "Managers/MeshManager.h"

void UDataManager::Initialize(
    const FIntVector &InWorldSizeInChunks, const int32 InChunkSize,
    const int32 InVoxelSize,
    const TSubclassOf<AVoxelChunkActor> &InVoxelChunkActor) {
  const AVoxelWorld *VoxelWorld = Cast<AVoxelWorld>(GetOuter());
  DebugManager = VoxelWorld->GetDebugManager();
  MeshManager = VoxelWorld->GetMeshManager();
  WorldSizeInChunks = InWorldSizeInChunks;
  ChunkSize = InChunkSize;
  VoxelSize = InVoxelSize;
  ChunkVolume = ChunkSize * ChunkSize * ChunkSize;
  VoxelChunk = InVoxelChunkActor;

  Chunks.Empty();
}

void UDataManager::LoadChunk(const FIntVector &ChunkCoord) {
  if (Chunks.Contains(ChunkCoord)) {
    return;
  }

  if (!VoxelChunk) {
    return;
  }

  FVoxelChunk NewChunk(ChunkSize);

  UWorld *World = GetWorld();
  if (!World) {
    return;
  }

  FActorSpawnParameters SpawnParams;
  SpawnParams.Owner = Cast<AActor>(GetOuter());

  AVoxelChunkActor *NewVoxelChunk = World->SpawnActor<AVoxelChunkActor>(
      VoxelChunk, GlobalToWorldCoord(ChunkToGlobalCoord(ChunkCoord)),
      FRotator::ZeroRotator, SpawnParams);

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

void UDataManager::UnloadChunk(const FIntVector &ChunkCoord) {
  if (const FVoxelChunk *Chunk = Chunks.Find(ChunkCoord)) {
    if (Chunk->VoxelChunkActor) {
      Chunk->VoxelChunkActor->Destroy();
    }
    Chunks.Remove(ChunkCoord);
  }
}

const FVoxelBaseData *
UDataManager::GetVoxelData(const FIntVector &GlobalCoord) const {
  if (const FVoxelChunk *Chunk = Chunks.Find(GlobalToChunkCoord(GlobalCoord))) {
    return Chunk->GetVoxelData(GlobalCoordToIndex(GlobalCoord));
  }
  return new FVoxelVoidData();
}

void UDataManager::ModifyVoxelData(
    const FIntVector &GlobalCoord,
    const TFunction<void(FVoxelBaseData *)> &Modifier, const bool bAutoDebug) {
  if (const FVoxelChunk *Chunk = Chunks.Find(GlobalToChunkCoord(GlobalCoord))) {
    Modifier(Chunk->GetVoxelData(GlobalCoordToIndex(GlobalCoord)));

    if (DebugManager && bAutoDebug) {
      DebugManager->SetDebugVoxel(GlobalCoord);
    }

    if (MeshManager) {
      MeshManager->SetDirtyChunk(GlobalCoord);
    }
  }
}

void UDataManager::SetVoxelData(const FIntVector &GlobalCoord,
                                FVoxelBaseData *VoxelData,
                                const bool bAutoDebug) {
  if (const FVoxelChunk *Chunk = Chunks.Find(GlobalToChunkCoord(GlobalCoord))) {
    Chunk->SetVoxelData(GlobalCoordToIndex(GlobalCoord), VoxelData);

    if (DebugManager && bAutoDebug) {
      DebugManager->SetDebugVoxel(GlobalCoord);
    }

    if (MeshManager) {
      MeshManager->SetDirtyChunk(GlobalCoord);
    }
  }
}

bool UDataManager::IsChunk(const FIntVector &ChunkCoord) const {
  return Chunks.Contains(ChunkCoord);
}

FVoxelChunk *UDataManager::GetChunk(const FIntVector &ChunkCoord) {
  return Chunks.Find(ChunkCoord);
}

FIntVector
UDataManager::GlobalToChunkCoord(const FIntVector &GlobalCoord) const {
  return FIntVector(
      FMath::FloorToInt(static_cast<float>(GlobalCoord.X) / ChunkSize),
      FMath::FloorToInt(static_cast<float>(GlobalCoord.Y) / ChunkSize),
      FMath::FloorToInt(static_cast<float>(GlobalCoord.Z) / ChunkSize));
}

FIntVector
UDataManager::ChunkToGlobalCoord(const FIntVector &ChunkCoord) const {
  return ChunkCoord * ChunkSize;
}

FIntVector
UDataManager::GlobalToLocalCoord(const FIntVector &GlobalCoord) const {
  return GlobalCoord % ChunkSize;
}

int32 UDataManager::GlobalCoordToIndex(const FIntVector &GlobalCoord) const {
  return LocalCoordToIndex(GlobalToLocalCoord(GlobalCoord));
}

FIntVector
UDataManager::LocalToGlobalCoord(const FIntVector &LocalCoord,
                                 const FIntVector &ChunkCoord) const {
  return ChunkToGlobalCoord(ChunkCoord) + LocalCoord;
}

FIntVector UDataManager::WorldToGlobalCoord(const FVector &WorldCoord) const {
  return FIntVector(FMath::RoundToInt(WorldCoord.X / VoxelSize),
                    FMath::RoundToInt(WorldCoord.Y / VoxelSize),
                    FMath::RoundToInt(WorldCoord.Z / VoxelSize));
}

FVector UDataManager::GlobalToWorldCoord(const FIntVector &GlobalCoord) const {
  return FVector(GlobalCoord) * VoxelSize;
}

int32 UDataManager::LocalCoordToIndex(const FIntVector &LocalCoord) const {
  return LocalCoord.X + LocalCoord.Y * ChunkSize +
         LocalCoord.Z * ChunkSize * ChunkSize;
}