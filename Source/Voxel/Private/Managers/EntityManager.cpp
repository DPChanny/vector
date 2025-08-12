#include "Managers/EntityManager.h"

#include "Actors/EntityChunkActor.h"
#include "DataAssets/VoxelEntityDataAsset.h"
#include "Managers/DataManager.h"

void UEntityManager::InitializeComponent() {
  Super::InitializeComponent();

  if (const AActor* Owner = GetOwner()) {
    DataManager = Owner->GetComponentByClass<UDataManager>();
  }
}

void UEntityManager::OnEntityDataCreated(const FIntVector& GlobalCoord,
                                         const FVoxelEntityData& EntityData) {
  if (EntityToChunk.Contains(GlobalCoord)) {
    return;
  }

  TSet<TObjectPtr<AEntityChunkActor>> ChunkableChunks;

  for (const FIntVector& NeighborOffset : NeighborOffsets) {
    if (const TObjectPtr<AEntityChunkActor>* FoundChunk =
            EntityToChunk.Find(GlobalCoord + NeighborOffset)) {
      if (const FVoxelEntityData* NeighborEntityData =
              dynamic_cast<const FVoxelEntityData*>(
                  DataManager->GetVoxelData(GlobalCoord + NeighborOffset))) {
        if (EntityData.IsChunkableWith(NeighborEntityData)) {
          ChunkableChunks.Add(*FoundChunk);
        }
      }
    }
  }

  TObjectPtr<AEntityChunkActor> TargetChunk;

  if (ChunkableChunks.IsEmpty()) {
    TargetChunk = CreateEntityChunk(EntityData);
  } else {
    TargetChunk = ChunkableChunks.Array()[0];
    ChunkableChunks.Remove(TargetChunk);

    for (TObjectPtr ChunkToMerge : ChunkableChunks) {
      TArray<FIntVector> VoxelsToMove =
          ChunkToMerge->GetManagedVoxels().Array();

      for (const FIntVector& VoxelCoord : VoxelsToMove) {
        if (const FVoxelEntityData* VoxelEntityData =
                dynamic_cast<const FVoxelEntityData*>(
                    DataManager->GetVoxelData(VoxelCoord))) {
          ChunkToMerge->RemoveEntity(VoxelCoord, *VoxelEntityData);
          TargetChunk->AddEntity(VoxelCoord, *VoxelEntityData);
          EntityToChunk[VoxelCoord] = TargetChunk;
        }
      }

      EntityChunks.Remove(ChunkToMerge);
      ChunkToMerge->Destroy();
    }
  }

  TargetChunk->AddEntity(GlobalCoord, EntityData);
  EntityToChunk.Add(GlobalCoord, TargetChunk);
}

void UEntityManager::OnEntityDataDestroyed(const FIntVector& GlobalCoord,
                                           const FVoxelEntityData& EntityData) {
  if (!EntityToChunk.Contains(GlobalCoord)) {
    return;
  }

  const TObjectPtr OwningChunk =
      EntityToChunk.FindAndRemoveChecked(GlobalCoord);

  OwningChunk->RemoveEntity(GlobalCoord, EntityData);

  if (OwningChunk->IsEmpty()) {
    EntityChunks.Remove(OwningChunk);
    OwningChunk->Destroy();
  } else {
    UpdateEntityChunk(OwningChunk);
  }
}

void UEntityManager::OnEntityDataModified(const FIntVector& GlobalCoord,
                                          const FVoxelEntityData& EntityData) {
  if (const TObjectPtr<AEntityChunkActor>* FoundChunk =
          EntityToChunk.Find(GlobalCoord)) {
    if (*FoundChunk) {
      (*FoundChunk)->OnEntityDataModified(GlobalCoord, EntityData);
    }
  }
}

UEntityManager::UEntityManager() {
  bWantsInitializeComponent = true;
}

TObjectPtr<AEntityChunkActor> UEntityManager::CreateEntityChunk(
    const FVoxelEntityData& EntityData) {
  const TObjectPtr<const UVoxelEntityDataAsset> EntityDataAsset =
      EntityData.GetEntityDataAsset();

  if (!EntityDataAsset) {
    return nullptr;
  }

  TObjectPtr<AEntityChunkActor> NewEntityChunkActor;

  if (EntityDataAsset->EntityChunkActorClass) {
    NewEntityChunkActor = GetWorld()->SpawnActor<AEntityChunkActor>(
        EntityDataAsset->EntityChunkActorClass);
  } else {
    NewEntityChunkActor = GetWorld()->SpawnActor<AEntityChunkActor>();
  }

  EntityChunks.Add(NewEntityChunkActor);

  return NewEntityChunkActor;
}

void UEntityManager::UpdateEntityChunk(
    const TObjectPtr<AEntityChunkActor>& OriginalChunk) {
  TArray<FIntVector> OriginalVoxels = OriginalChunk->GetManagedVoxels().Array();

  for (const FIntVector& VoxelCoord : OriginalVoxels) {
    if (const FVoxelEntityData* EntityData =
            dynamic_cast<const FVoxelEntityData*>(
                DataManager->GetVoxelData(VoxelCoord))) {
      OriginalChunk->RemoveEntity(VoxelCoord, *EntityData);
    }
    EntityToChunk.Remove(VoxelCoord);
  }

  EntityChunks.Remove(OriginalChunk);
  OriginalChunk->Destroy();

  TSet<FIntVector> VisitedVoxels;

  for (const FIntVector& VoxelCoord : OriginalVoxels) {
    if (VisitedVoxels.Contains(VoxelCoord)) {
      continue;
    }

    if (TSet<FIntVector> ChunkableVoxels;
        GetChunkableEntityCoords(VoxelCoord, VisitedVoxels, ChunkableVoxels)) {
      if (const FVoxelEntityData* EntityData =
              dynamic_cast<const FVoxelEntityData*>(
                  DataManager->GetVoxelData(VoxelCoord))) {
        TObjectPtr<AEntityChunkActor> NewChunk = CreateEntityChunk(*EntityData);

        for (const FIntVector& ChunkableVoxel : ChunkableVoxels) {
          if (const FVoxelEntityData* VoxelEntityData =
                  dynamic_cast<const FVoxelEntityData*>(
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
    const FIntVector& StartCoord, TSet<FIntVector>& VisitedCoords,
    TSet<FIntVector>& ChunkableEntityCoords) const {
  ChunkableEntityCoords.Reset();
  TQueue<FIntVector> ToVisit;

  const FVoxelEntityData* StartEntityData =
      dynamic_cast<const FVoxelEntityData*>(
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

    const FVoxelEntityData* CurrentEntityData =
        dynamic_cast<const FVoxelEntityData*>(
            DataManager->GetVoxelData(CurrentCoord));

    if (!CurrentEntityData ||
        !StartEntityData->IsChunkableWith(CurrentEntityData)) {
      continue;
    }

    VisitedCoords.Add(CurrentCoord);
    ChunkableEntityCoords.Add(CurrentCoord);

    for (const FIntVector& Offset : NeighborOffsets) {
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
