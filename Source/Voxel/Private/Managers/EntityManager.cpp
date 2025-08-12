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
  TSet<FIntVector> ProcessedVoxels;

  for (const FIntVector& GlobalCoord : DirtyEntityVoxels) {
    if (ProcessedVoxels.Contains(GlobalCoord)) {
      continue;
    }

    const FVoxelEntityData* EntityData = dynamic_cast<const FVoxelEntityData*>(
        DataManager->GetVoxelData(GlobalCoord));

    if (EntityData) {
      MergeEntityChunk(GlobalCoord, *EntityData);
    } else if (EntityToChunk.Contains(GlobalCoord)) {
      SplitEntityChunk(GlobalCoord, ProcessedVoxels);
    }

    ProcessedVoxels.Add(GlobalCoord);
  }

  DirtyEntityVoxels = DirtyEntityVoxels.Difference(ProcessedVoxels);
}

void UEntityManager::MergeEntityChunk(const FIntVector& GlobalCoord,
                                      const FVoxelEntityData& EntityData) {
  if (EntityToChunk.Contains(GlobalCoord)) {
    if (const TObjectPtr<AEntityChunkActor>* FoundEntityChunkActor =
            EntityToChunk.Find(GlobalCoord)) {
      const TObjectPtr<AEntityChunkActor> EntityChunkActor =
          *FoundEntityChunkActor;

      if (EntityChunkActor->IsChunkableWith(EntityData)) {
        EntityChunkActor->OnEntityModified(GlobalCoord, EntityData);
        return;
      }

      EntityChunkActor->RemoveEntity(GlobalCoord);
      EntityToChunk.Remove(GlobalCoord);

      if (EntityChunkActor->GetEntityCoords().IsEmpty()) {
        EntityChunkActors.Remove(EntityChunkActor);
        EntityChunkActor->Destroy();
      }
    }
  }

  TSet<TObjectPtr<AEntityChunkActor>> ChunkableEntityChunkActors;

  for (const FIntVector& Offset : NeighborOffsets) {
    const FIntVector NeighborCoord = GlobalCoord + Offset;
    if (const TObjectPtr<AEntityChunkActor>* FoundEntityChunkActor =
            EntityToChunk.Find(NeighborCoord)) {
      if ((*FoundEntityChunkActor)->IsChunkableWith(EntityData)) {
        ChunkableEntityChunkActors.Add(*FoundEntityChunkActor);
      }
    }
  }

  TObjectPtr<AEntityChunkActor> ToEntityChunkActor;

  if (!ChunkableEntityChunkActors.IsEmpty()) {
    ToEntityChunkActor = *ChunkableEntityChunkActors.begin();
    ChunkableEntityChunkActors.Remove(ToEntityChunkActor);

    for (const TObjectPtr<AEntityChunkActor>& FromEntityChunkActor :
         ChunkableEntityChunkActors) {
      for (const FIntVector& EntityCoord :
           FromEntityChunkActor->GetEntityCoords()) {
        if (const FVoxelEntityData* VoxelEntityData =
                dynamic_cast<const FVoxelEntityData*>(
                    DataManager->GetVoxelData(EntityCoord))) {
          ToEntityChunkActor->AddEntity(EntityCoord, *VoxelEntityData);
          EntityToChunk[EntityCoord] = ToEntityChunkActor;
        }
      }

      EntityChunkActors.Remove(FromEntityChunkActor);
      FromEntityChunkActor->Destroy();
    }
  } else {
    ToEntityChunkActor = ConstructEntityChunk(EntityData);
  }

  ToEntityChunkActor->AddEntity(GlobalCoord, EntityData);
  EntityToChunk.Add(GlobalCoord, ToEntityChunkActor);
}

void UEntityManager::SplitEntityChunk(const FIntVector& GlobalCoord,
                                      TSet<FIntVector>& ProcessedVoxels) {
  const TObjectPtr<AEntityChunkActor>* FoundEntityChunkActor =
      EntityToChunk.Find(GlobalCoord);
  if (!FoundEntityChunkActor) {
    return;
  }

  const TObjectPtr<AEntityChunkActor> EntityChunkActor = *FoundEntityChunkActor;
  TSet<FIntVector> EntityCoords = EntityChunkActor->GetEntityCoords();

  EntityChunkActor->RemoveEntity(GlobalCoord);
  EntityToChunk.Remove(GlobalCoord);
  EntityCoords.Remove(GlobalCoord);

  if (EntityCoords.IsEmpty()) {
    EntityChunkActors.Remove(EntityChunkActor);
    EntityChunkActor->Destroy();
    return;
  }

  EntityChunkActors.Remove(EntityChunkActor);

  for (const FIntVector& EntityCoord : EntityCoords) {
    EntityToChunk.Remove(EntityCoord);
  }

  EntityChunkActor->Destroy();

  for (const FIntVector& VoxelCoord : EntityCoords) {
    if (ProcessedVoxels.Contains(VoxelCoord)) {
      continue;
    }

    if (const FVoxelEntityData* VoxelEntityData =
            dynamic_cast<const FVoxelEntityData*>(
                DataManager->GetVoxelData(VoxelCoord))) {
      MergeEntityChunk(VoxelCoord, *VoxelEntityData);
      ProcessedVoxels.Add(VoxelCoord);
    }
  }
}

UEntityManager::UEntityManager() {
  bWantsInitializeComponent = true;
}

TObjectPtr<AEntityChunkActor> UEntityManager::ConstructEntityChunk(
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

  EntityChunkActors.Add(NewEntityChunkActor);

  return NewEntityChunkActor;
}
