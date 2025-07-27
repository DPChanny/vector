#include "Managers/EntityManager.h"

#include "Actors/VoxelWorldActor.h"
#include "DataAssets/VoxelEntityDataAsset.h"
#include "EntityChunk.h"
#include "EntityComponent.h"
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

void UEntityManager::OnEntityDataCreated(const FIntVector &GlobalCoord,
                                         const FVoxelEntityData &EntityData) {
  if (EntityToChunk.Contains(GlobalCoord)) {
    return;
  }

  TSet<TObjectPtr<UEntityChunk>> ChunkableChunks;

  for (const FIntVector &NeighborOffset : NeighborOffsets) {
    if (const TObjectPtr<UEntityChunk> *FoundChunk =
            EntityToChunk.Find(GlobalCoord + NeighborOffset)) {
      if (const FVoxelEntityData *NeighborEntityData =
              dynamic_cast<const FVoxelEntityData *>(
                  DataManager->GetVoxelData(GlobalCoord + NeighborOffset))) {
        if (EntityData.IsChunkableWith(NeighborEntityData)) {
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
        if (const FVoxelEntityData *VoxelEntityData =
                dynamic_cast<const FVoxelEntityData *>(
                    DataManager->GetVoxelData(VoxelCoord))) {
          ChunkToMerge->RemoveEntity(VoxelCoord, *VoxelEntityData);
          TargetChunk->AddEntity(VoxelCoord, *VoxelEntityData);
          EntityToChunk[VoxelCoord] = TargetChunk;
        }
      }

      EntityChunks.Remove(ChunkToMerge);
    }
  }

  TargetChunk->AddEntity(GlobalCoord, EntityData);
  EntityToChunk.Add(GlobalCoord, TargetChunk);
}
void UEntityManager::OnEntityDataDestroyed(const FIntVector &GlobalCoord,
                                           const FVoxelEntityData &EntityData) {
  if (!EntityToChunk.Contains(GlobalCoord)) {
    return;
  }

  UEntityChunk *OwningChunk = EntityToChunk.FindAndRemoveChecked(GlobalCoord);

  OwningChunk->RemoveEntity(GlobalCoord, EntityData);

  if (OwningChunk->IsEmpty()) {
    EntityChunks.Remove(OwningChunk);
  } else {
    UpdateEntityChunk(OwningChunk);
  }
}

void UEntityManager::OnEntityDataModified(const FIntVector &GlobalCoord,
                                          const FVoxelEntityData &EntityData) {
  if (const TObjectPtr<UEntityChunk> *FoundChunk =
          EntityToChunk.Find(GlobalCoord)) {
    if (*FoundChunk) {
      (*FoundChunk)->OnEntityDataModified(GlobalCoord, EntityData);
    }
  }
}

TObjectPtr<UEntityChunk>
UEntityManager::CreateEntityChunk(const FVoxelEntityData &EntityData) {
  UEntityChunk *NewChunk = nullptr;

  if (const UVoxelEntityDataAsset *EntityDataAsset =
          EntityData.GetEntityDataAsset()) {
    if (EntityDataAsset->EntityChunkClass) {
      NewChunk =
          NewObject<UEntityChunk>(this, EntityDataAsset->EntityChunkClass);
    }

    if (!NewChunk) {
      NewChunk = NewObject<UEntityChunk>(this);
    }

    for (const TSubclassOf<UEntityComponent> &EntityComponentClass :
         EntityDataAsset->EntityComponentClasses) {
      if (EntityComponentClass) {
        NewChunk->AddComponent(
            NewObject<UEntityComponent>(NewChunk, EntityComponentClass));
      }
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
    if (const FVoxelEntityData *EntityData =
            dynamic_cast<const FVoxelEntityData *>(
                DataManager->GetVoxelData(VoxelCoord))) {
      OriginalChunk->RemoveEntity(VoxelCoord, *EntityData);
    }
    EntityToChunk.Remove(VoxelCoord);
  }

  EntityChunks.Remove(OriginalChunk);

  TSet<FIntVector> VisitedVoxels;

  for (const FIntVector &VoxelCoord : OriginalVoxels) {
    if (VisitedVoxels.Contains(VoxelCoord)) {
      continue;
    }

    if (TSet<FIntVector> ChunkableVoxels;
        GetChunkableEntityCoords(VoxelCoord, VisitedVoxels, ChunkableVoxels)) {
      if (const FVoxelEntityData *EntityData =
              dynamic_cast<const FVoxelEntityData *>(
                  DataManager->GetVoxelData(VoxelCoord))) {
        TObjectPtr<UEntityChunk> NewChunk = CreateEntityChunk(*EntityData);

        for (const FIntVector &ChunkableVoxel : ChunkableVoxels) {
          if (const FVoxelEntityData *VoxelEntityData =
                  dynamic_cast<const FVoxelEntityData *>(
                      DataManager->GetVoxelData(ChunkableVoxel))) {
            NewChunk->AddEntity(ChunkableVoxel, *VoxelEntityData);
            EntityToChunk.Add(ChunkableVoxel, NewChunk);
          }
        }
      }
    }
  }
}

bool UEntityManager::GetChunkableEntityCoords(
    const FIntVector &StartCoord, TSet<FIntVector> &VisitedCoords,
    TSet<FIntVector> &ChunkableEntityCoords) const {
  ChunkableEntityCoords.Reset();
  TQueue<FIntVector> ToVisit;

  const FVoxelEntityData *StartEntityData =
      dynamic_cast<const FVoxelEntityData *>(
          DataManager->GetVoxelData(StartCoord));

  if (!StartEntityData) {
    return false;
  }

  ToVisit.Enqueue(StartCoord);

  while (!ToVisit.IsEmpty()) {
    FIntVector CurrentCoord;
    ToVisit.Dequeue(CurrentCoord);

    if (VisitedCoords.Contains(CurrentCoord)) {
      continue;
    }

    const FVoxelEntityData *CurrentEntityData =
        dynamic_cast<const FVoxelEntityData *>(
            DataManager->GetVoxelData(CurrentCoord));

    if (!CurrentEntityData ||
        !StartEntityData->IsChunkableWith(CurrentEntityData)) {
      continue;
    }

    VisitedCoords.Add(CurrentCoord);
    ChunkableEntityCoords.Add(CurrentCoord);

    for (const FIntVector &Offset : NeighborOffsets) {
      if (const FIntVector NeighborCoord = CurrentCoord + Offset;
          !VisitedCoords.Contains(NeighborCoord)) {
        if (FVoxelEntityData::IsEntity(
                DataManager->GetVoxelData(NeighborCoord))) {
          ToVisit.Enqueue(NeighborCoord);
        }
      }
    }
  }

  return !ChunkableEntityCoords.IsEmpty();
}
