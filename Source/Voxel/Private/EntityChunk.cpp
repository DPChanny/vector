#include "EntityChunk.h"
#include "DataAssets/VoxelEntityDataAsset.h"
#include "EntityComponent.h"
#include "Managers/DataManager.h"

void UEntityChunk::Initialize(const TObjectPtr<UDataManager> InDataManager) {
  if (InDataManager) {
    DataManager = InDataManager;
  }
}

void UEntityChunk::Tick(const float DeltaTime) {
  for (UEntityComponent *Component : Components) {
    if (Component) {
      Component->Tick(DeltaTime);
    }
  }
}

void UEntityChunk::AddEntity(const FIntVector &VoxelCoord,
                             const FVoxelEntityData &EntityData) {
  ManagedVoxels.Add(VoxelCoord);
  UpdateCenterOfMass();

  for (const TObjectPtr<UEntityComponent> &Component : Components) {
    if (Component) {
      Component->OnEntityAdded(VoxelCoord, EntityData);
    }
  }
}

void UEntityChunk::RemoveEntity(const FIntVector &VoxelCoord,
                                const FVoxelEntityData &EntityData) {
  ManagedVoxels.Remove(VoxelCoord);
  UpdateCenterOfMass();

  for (const TObjectPtr<UEntityComponent> &Component : Components) {
    if (Component) {
      Component->OnEntityRemoved(VoxelCoord, EntityData);
    }
  }
}

bool UEntityChunk::IsEmpty() const { return ManagedVoxels.IsEmpty(); }

void UEntityChunk::OnEntityDataModified(const FIntVector &VoxelCoord,
                                        const FVoxelEntityData &EntityData) {
  UpdateCenterOfMass();
  for (const TObjectPtr<UEntityComponent> &Component : Components) {
    if (Component) {
      Component->OnEntityDataModified(VoxelCoord, EntityData);
    }
  }
}

void UEntityChunk::AddComponent(const TObjectPtr<UEntityComponent> &Component) {
  if (Component) {
    Components.Add(Component);
  }
}

void UEntityChunk::UpdateCenterOfMass() {
  CenterOfMass = FVector::ZeroVector;
  for (const FIntVector &VoxelCoord : ManagedVoxels) {
    if (const FVoxelEntityData *EntityData =
            dynamic_cast<const FVoxelEntityData *>(
                DataManager->GetVoxelData(VoxelCoord))) {
      CenterOfMass += FVector(DataManager->GlobalToWorldCoord(VoxelCoord));
    }
  }
  CenterOfMass /= ManagedVoxels.Num();
}
