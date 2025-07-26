#include "Managers/DataManager.h"

#include "Actors/VoxelChunkActor.h"
#include "Actors/VoxelWorldActor.h"
#include "DataAssets/VoxelBaseDataAsset.h"
#include "DataAssets/VoxelEntityDataAsset.h"
#include "Managers/DebugManager.h"
#include "Managers/EntityManager.h"
#include "Managers/MeshManager.h"

void UDataManager::Initialize(
    const FIntVector &InWorldSizeInChunks, const int32 InChunkSize,
    const int32 InVoxelSize,
    const TSubclassOf<AVoxelChunkActor> &InVoxelChunkActor) {
  const AVoxelWorldActor *VoxelWorld = Cast<AVoxelWorldActor>(GetOuter());
  DebugManager = VoxelWorld->GetDebugManager();
  MeshManager = VoxelWorld->GetMeshManager();
  EntityManager = VoxelWorld->GetEntityManager();
  WorldSizeInChunks = InWorldSizeInChunks;
  ChunkSize = InChunkSize;
  VoxelSize = InVoxelSize;
  ChunkVolume = ChunkSize * ChunkSize * ChunkSize;
  VoxelChunk = InVoxelChunkActor;

  VoxelChunks.Empty();
}

void UDataManager::LoadVoxelChunk(const FIntVector &ChunkCoord) {
  if (VoxelChunks.Contains(ChunkCoord)) {
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

    VoxelChunks.Add(ChunkCoord, MoveTemp(NewChunk));
  }
}

void UDataManager::UnloadVoxelChunk(const FIntVector &ChunkCoord) {
  if (const FVoxelChunk *Chunk = VoxelChunks.Find(ChunkCoord)) {
    if (Chunk->VoxelChunkActor) {
      Chunk->VoxelChunkActor->Destroy();
    }
    VoxelChunks.Remove(ChunkCoord);
  }
}

const FVoxelBaseData *
UDataManager::GetVoxelData(const FIntVector &GlobalCoord) const {
  if (const FVoxelChunk *Chunk =
          VoxelChunks.Find(GlobalToChunkCoord(GlobalCoord))) {
    return Chunk->GetVoxelData(GlobalCoordToIndex(GlobalCoord));
  }
  return nullptr;
}

void UDataManager::ModifyVoxelData(
    const FIntVector &GlobalCoord,
    const TFunction<void(FVoxelBaseData *)> &Modifier, const bool bAutoDebug) {
  if (const FVoxelChunk *Chunk =
          VoxelChunks.Find(GlobalToChunkCoord(GlobalCoord))) {
    Modifier(Chunk->GetVoxelData(GlobalCoordToIndex(GlobalCoord)));

    if (EntityManager) {
      EntityManager->OnEntityDataModified(GlobalCoord);
    }

    if (DebugManager && bAutoDebug) {
      DebugManager->SetDebugVoxel(GlobalCoord);
    }

    if (MeshManager) {
      MeshManager->SetDirtyChunk(GlobalCoord);
    }
  }
}

void UDataManager::SetVoxelData(const FIntVector &GlobalCoord,
                                FVoxelBaseData *NewVoxelData,
                                const bool bAutoDebug) {
  if (const FVoxelChunk *Chunk =
          VoxelChunks.Find(GlobalToChunkCoord(GlobalCoord))) {
    const FVoxelBaseData *OldData =
        Chunk->GetVoxelData(GlobalCoordToIndex(GlobalCoord));

    Chunk->SetVoxelData(GlobalCoordToIndex(GlobalCoord), NewVoxelData);

    if (EntityManager) {
      if (const FVoxelEntityData *NewEntityData =
              dynamic_cast<const FVoxelEntityData *>(NewVoxelData)) {
        EntityManager->OnEntityDataCreated(GlobalCoord, NewEntityData);
      } else if (FVoxelEntityData::IsEntity(OldData)) {
        EntityManager->OnEntityDataDestroyed(GlobalCoord);
      }
    }

    delete OldData;

    if (DebugManager && bAutoDebug) {
      DebugManager->SetDebugVoxel(GlobalCoord);
    }

    if (MeshManager) {
      MeshManager->SetDirtyChunk(GlobalCoord);
    }
  }
}

bool UDataManager::IsVoxelChunkLoaded(const FIntVector &ChunkCoord) const {
  return VoxelChunks.Contains(ChunkCoord);
}

FVoxelChunk *UDataManager::GetVoxelChunk(const FIntVector &ChunkCoord) {
  return VoxelChunks.Find(ChunkCoord);
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