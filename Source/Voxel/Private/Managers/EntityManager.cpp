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

void UEntityManager::SetDirtyEntity(const FIntVector& GlobalCoord) {
  DirtyEntityVoxels.Add(GlobalCoord);
}

void UEntityManager::OnEntityModified(const FIntVector& GlobalCoord,
                                      const FVoxelEntityData& EntityData) {
  if (const TObjectPtr<AEntityChunkActor>* FoundChunk =
          EntityToChunk.Find(GlobalCoord)) {
    (*FoundChunk)->OnEntityModified(GlobalCoord, EntityData);
  }
}

void UEntityManager::FlushDirtyEntities() {
  const TSet<FIntVector> ProcessedVoxels;

  for (const FIntVector& GlobalCoord : DirtyEntityVoxels) {
    if (ProcessedVoxels.Contains(GlobalCoord)) {
      continue;
    }

    const FVoxelEntityData* EntityData = dynamic_cast<const FVoxelEntityData*>(
        DataManager->GetVoxelData(GlobalCoord));

    if (EntityData) {
      TSet<FIntVector> MergedVoxels;
      MergeEntityChunk(GlobalCoord, *EntityData, ProcessedVoxels, MergedVoxels);
    } else {
      TSet<FIntVector> SplitVoxels;
      SplitEntityChunk(GlobalCoord, ProcessedVoxels, SplitVoxels);
    }
  }

  DirtyEntityVoxels = DirtyEntityVoxels.Difference(ProcessedVoxels);
}

void UEntityManager::MergeEntityChunk(const FIntVector& GlobalCoord,
                                      const FVoxelEntityData& EntityData,
                                      const TSet<FIntVector>& ProcessedVoxels,
                                      TSet<FIntVector>& MergedVoxels) {
  if (EntityToChunk.Contains(GlobalCoord)) {
    MergedVoxels.Add(GlobalCoord);
    return;
  }

  TObjectPtr<AEntityChunkActor> TargetChunk = nullptr;
  TSet<FIntVector> ChunkableVoxels;
  TSet<FIntVector> VisitedVoxels;

  for (const FIntVector& Offset : NeighborOffsets) {
    const FIntVector NeighborCoord = GlobalCoord + Offset;
    if (const TObjectPtr<AEntityChunkActor>* FoundChunk =
            EntityToChunk.Find(NeighborCoord)) {
      if ((*FoundChunk)->IsChunkableWith(EntityData)) {
        TargetChunk = *FoundChunk;
        break;
      }
    }
  }

  if (!TargetChunk) {
    TargetChunk = GetEntityChunk(EntityData);
  }

  if (GetChunkableEntityCoords(GlobalCoord, VisitedVoxels, ChunkableVoxels)) {
    TMap<TObjectPtr<AEntityChunkActor>, TSet<FIntVector>> ChunksToUpdate;

    for (const FIntVector& VoxelCoord : ChunkableVoxels) {
      if (ProcessedVoxels.Contains(VoxelCoord)) {
        continue;
      }

      if (const TObjectPtr<AEntityChunkActor>* ExistingChunk =
              EntityToChunk.Find(VoxelCoord)) {
        if (*ExistingChunk != TargetChunk) {
          ChunksToUpdate.FindOrAdd(*ExistingChunk).Add(VoxelCoord);
        }
      }
    }

    for (const auto& [ChunkToUpdate, VoxelsToRemove] : ChunksToUpdate) {
      for (const FIntVector& VoxelCoord : VoxelsToRemove) {
        if (const FVoxelEntityData* VoxelEntityData =
                dynamic_cast<const FVoxelEntityData*>(
                    DataManager->GetVoxelData(VoxelCoord))) {
          ChunkToUpdate->RemoveEntity(VoxelCoord);
          EntityToChunk.Remove(VoxelCoord);
        }
      }

      if (ChunkToUpdate->IsEmpty()) {
        EntityChunks.Remove(ChunkToUpdate);
        ChunkToUpdate->Destroy();
      }
    }

    for (const FIntVector& VoxelCoord : ChunkableVoxels) {
      if (ProcessedVoxels.Contains(VoxelCoord)) {
        continue;
      }

      if (const FVoxelEntityData* VoxelEntityData =
              dynamic_cast<const FVoxelEntityData*>(
                  DataManager->GetVoxelData(VoxelCoord))) {
        TargetChunk->AddEntity(VoxelCoord, *VoxelEntityData);
        EntityToChunk.Add(VoxelCoord, TargetChunk);
      }
    }

    MergedVoxels = ChunkableVoxels;
  }
}

void UEntityManager::SplitEntityChunk(const FIntVector& GlobalCoord,
                                      const TSet<FIntVector>& ProcessedVoxels,
                                      TSet<FIntVector>& SplitVoxels) {
  const TObjectPtr<AEntityChunkActor>* FoundChunk =
      EntityToChunk.Find(GlobalCoord);
  if (!FoundChunk) {
    SplitVoxels.Add(GlobalCoord);
    return;
  }

  TObjectPtr<AEntityChunkActor> OriginalChunk = *FoundChunk;
  TSet<FIntVector> OriginalVoxels = OriginalChunk->GetEntities();

  if (const FVoxelEntityData* EntityData =
          dynamic_cast<const FVoxelEntityData*>(
              DataManager->GetVoxelData(GlobalCoord))) {
    OriginalChunk->RemoveEntity(GlobalCoord);
  }
  EntityToChunk.Remove(GlobalCoord);
  OriginalVoxels.Remove(GlobalCoord);

  if (OriginalVoxels.IsEmpty()) {
    EntityChunks.Remove(OriginalChunk);
    OriginalChunk->Destroy();
    SplitVoxels.Add(GlobalCoord);
    return;
  }

  TSet<FIntVector> AllProcessedVoxels = ProcessedVoxels;
  AllProcessedVoxels.Add(GlobalCoord);

  for (const FIntVector& VoxelCoord : OriginalVoxels) {
    if (const FVoxelEntityData* EntityData =
            dynamic_cast<const FVoxelEntityData*>(
                DataManager->GetVoxelData(VoxelCoord))) {
      OriginalChunk->RemoveEntity(VoxelCoord);
    }
    EntityToChunk.Remove(VoxelCoord);
  }
  EntityChunks.Remove(OriginalChunk);
  OriginalChunk->Destroy();

  TSet<FIntVector> VisitedForSplit;
  for (const FIntVector& VoxelCoord : OriginalVoxels) {
    if (VisitedForSplit.Contains(VoxelCoord) ||
        AllProcessedVoxels.Contains(VoxelCoord)) {
      continue;
    }

    TSet<FIntVector> ChunkableVoxels;
    if (GetChunkableEntityCoords(VoxelCoord, VisitedForSplit,
                                 ChunkableVoxels)) {
      if (const FVoxelEntityData* EntityData =
              dynamic_cast<const FVoxelEntityData*>(
                  DataManager->GetVoxelData(VoxelCoord))) {
        TObjectPtr<AEntityChunkActor> NewChunk = GetEntityChunk(*EntityData);

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

  SplitVoxels = OriginalVoxels;
  SplitVoxels.Add(GlobalCoord);
}

UEntityManager::UEntityManager() {
  bWantsInitializeComponent = true;
}

TObjectPtr<AEntityChunkActor> UEntityManager::GetEntityChunk(
    const FVoxelEntityData& EntityData) {
  const TObjectPtr<const UVoxelEntityDataAsset> EntityDataAsset =
      EntityData.GetEntityDataAsset();

  if (!EntityDataAsset) {
    return nullptr;
  }

  const TObjectPtr<UWorld> World = GetWorld();
  if (!World) {
    return nullptr;
  }

  TObjectPtr<AEntityChunkActor> NewEntityChunkActor;

  if (EntityDataAsset->EntityChunkActorClass) {
    NewEntityChunkActor = World->SpawnActor<AEntityChunkActor>(
        EntityDataAsset->EntityChunkActorClass);
  } else {
    NewEntityChunkActor = World->SpawnActor<AEntityChunkActor>();
  }

  EntityChunks.Add(NewEntityChunkActor);

  return NewEntityChunkActor;
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
