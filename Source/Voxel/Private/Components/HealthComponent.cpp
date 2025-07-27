#include "Components/HealthComponent.h"
#include "DataAssets/VoxelEntityDataAsset.h"

void UHealthComponent::OnEntityAdded(const FIntVector &VoxelCoord,
                                     const FVoxelEntityData &Data) {
  TotalHealth += Data.GetEntityDataAsset()->MaxDurability;
  CurrentHealth += Data.Durability;
}

void UHealthComponent::OnEntityRemoved(const FIntVector &VoxelCoord,
                                       const FVoxelEntityData &Data) {
  TotalHealth -= Data.GetEntityDataAsset()->MaxDurability;
  CurrentHealth -= Data.Durability;
}
