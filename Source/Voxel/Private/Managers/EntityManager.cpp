#include "Managers/EntityManager.h"

#include "Actors/VoxelWorldActor.h"
#include "DataAssets/VoxelEntityDataAsset.h"
#include "EntityChunk.h"
#include "Managers/DataManager.h"

void UEntityManager::Initialize() {
  if (const TObjectPtr<AVoxelWorldActor> VoxelWorld =
          Cast<AVoxelWorldActor>(GetOuter())) {
    DataManager = VoxelWorld->GetDataManager();
  }
}

void UEntityManager::Tick(const float DeltaTime) {
  for (const TObjectPtr Chunk : EntityChunks) {
    if (Chunk) {
      Chunk->Tick(DeltaTime);
    }
  }
}

void UEntityManager::OnEntityCreated(const FIntVector &GlobalCoord,
                                     const FVoxelEntityData *EntityData) {
  if (EntityToChunk.Contains(GlobalCoord) || !EntityData) {
    return;
  }

  TSet<TObjectPtr<UEntityChunk>> ChunkableChunks;

  const FIntVector NeighborOffsets[] = {
      FIntVector(1, 0, 0),  FIntVector(-1, 0, 0), FIntVector(0, 1, 0),
      FIntVector(0, -1, 0), FIntVector(0, 0, 1),  FIntVector(0, 0, -1)};

  for (const FIntVector &NeighborOffset : NeighborOffsets) {
    if (const TObjectPtr<UEntityChunk> *FoundChunk =
            EntityToChunk.Find(GlobalCoord + NeighborOffset)) {
      if (const FVoxelEntityData *NeighborEntityData =
              dynamic_cast<const FVoxelEntityData *>(
                  DataManager->GetVoxelData(GlobalCoord + NeighborOffset))) {
        if (EntityData->IsChunkableWith(NeighborEntityData)) {
          ChunkableChunks.Add(*FoundChunk);
        }
      }
    }
  }

  TObjectPtr<UEntityChunk> TargetChunk;

  if (ChunkableChunks.IsEmpty()) {
    TargetChunk = CreateEntityChunk(EntityData);
  } else {
    TargetChunk = ChunkableChunks.Array()[0];
    ChunkableChunks.Remove(TargetChunk);

    for (UEntityChunk *ChunkToMerge : ChunkableChunks) {
      TArray<FIntVector> VoxelsToMove =
          ChunkToMerge->GetManagedVoxels().Array();

      for (const FIntVector &VoxelCoord : VoxelsToMove) {
        ChunkToMerge->RemoveEntity(VoxelCoord);
        TargetChunk->AddEntity(VoxelCoord);
        EntityToChunk[VoxelCoord] = TargetChunk;
      }

      EntityChunks.Remove(ChunkToMerge);
    }
  }

  EntityToChunk.Add(GlobalCoord, TargetChunk)->AddEntity(GlobalCoord);
}

void UEntityManager::OnEntityDestroyed(const FIntVector &GlobalCoord) {
  if (!EntityToChunk.Contains(GlobalCoord)) {
    return;
  }

  UEntityChunk *OwningChunk = EntityToChunk.FindAndRemoveChecked(GlobalCoord);
  OwningChunk->RemoveEntity(GlobalCoord);

  if (OwningChunk->IsEmpty()) {
    EntityChunks.Remove(OwningChunk);
  } else {
    UpdateEntityChunk(OwningChunk);
  }
}

void UEntityManager::OnEntityModified(const FIntVector &GlobalCoord) {
  if (const TObjectPtr<UEntityChunk> *FoundChunk =
          EntityToChunk.Find(GlobalCoord)) {
    if (*FoundChunk) {
      (*FoundChunk)->OnVoxelDataModified(GlobalCoord);
    }
  }
}

TObjectPtr<UEntityChunk>
UEntityManager::CreateEntityChunk(const FVoxelEntityData *EntityData) {
  UEntityChunk *NewChunk = nullptr;

  if (const UVoxelEntityDataAsset *EntityDataAsset =
          EntityData->GetEntityDataAsset()) {
    if (EntityDataAsset->EntityChunkClass) {
      NewChunk =
          NewObject<UEntityChunk>(this, EntityDataAsset->EntityChunkClass);
    }
  }

  if (!NewChunk) {
    NewChunk = NewObject<UEntityChunk>(this);
  }

  EntityChunks.Add(NewChunk);

  return NewChunk;
}

void UEntityManager::UpdateEntityChunk(
    const TObjectPtr<UEntityChunk> &OriginalChunk) {
  TArray<FIntVector> OriginalVoxels = OriginalChunk->GetManagedVoxels().Array();

  for (const FIntVector &VoxelCoord : OriginalVoxels) {
    OriginalChunk->RemoveEntity(VoxelCoord);
    EntityToChunk.Remove(VoxelCoord);
  }

  EntityChunks.Remove(OriginalChunk);

  TSet<FIntVector> VisitedVoxels;

  for (const FIntVector &VoxelCoord : OriginalVoxels) {
    if (VisitedVoxels.Contains(VoxelCoord)) {
      continue;
    }

    TSet<FIntVector> ChunkableVoxels =
        GetChunkableVoxels(VoxelCoord, VisitedVoxels);

    if (!ChunkableVoxels.IsEmpty()) {
      if (const FVoxelEntityData *EntityData =
              dynamic_cast<const FVoxelEntityData *>(
                  DataManager->GetVoxelData(VoxelCoord))) {
        TObjectPtr<UEntityChunk> NewChunk = CreateEntityChunk(EntityData);

        for (const FIntVector &ChunkableVoxel : ChunkableVoxels) {
          NewChunk->AddEntity(ChunkableVoxel);
          EntityToChunk.Add(ChunkableVoxel, NewChunk);
        }
      }
    }
  }
}

TSet<FIntVector>
UEntityManager::GetChunkableVoxels(const FIntVector &StartCoord,
                                   TSet<FIntVector> &VisitedVoxels) const {
  TSet<FIntVector> ConnectedVoxels;
  TQueue<FIntVector> ToVisit;

  const FVoxelEntityData *StartEntityData =
      dynamic_cast<const FVoxelEntityData *>(
          DataManager->GetVoxelData(StartCoord));

  if (!StartEntityData) {
    return ConnectedVoxels;
  }

  ToVisit.Enqueue(StartCoord);

  const FIntVector NeighborOffsets[] = {
      FIntVector(1, 0, 0),  FIntVector(-1, 0, 0), FIntVector(0, 1, 0),
      FIntVector(0, -1, 0), FIntVector(0, 0, 1),  FIntVector(0, 0, -1)};

  while (!ToVisit.IsEmpty()) {
    FIntVector CurrentCoord;
    ToVisit.Dequeue(CurrentCoord);

    if (VisitedVoxels.Contains(CurrentCoord)) {
      continue;
    }

    const FVoxelEntityData *CurrentEntityData =
        dynamic_cast<const FVoxelEntityData *>(
            DataManager->GetVoxelData(CurrentCoord));

    if (!CurrentEntityData ||
        !StartEntityData->IsChunkableWith(CurrentEntityData)) {
      continue;
    }

    VisitedVoxels.Add(CurrentCoord);
    ConnectedVoxels.Add(CurrentCoord);

    for (const FIntVector &Offset : NeighborOffsets) {
      const FIntVector NeighborCoord = CurrentCoord + Offset;
      if (!VisitedVoxels.Contains(NeighborCoord)) {
        if (FVoxelEntityData::IsEntity(
                DataManager->GetVoxelData(NeighborCoord))) {
          ToVisit.Enqueue(NeighborCoord);
        }
      }
    }
  }

  return ConnectedVoxels;
}
