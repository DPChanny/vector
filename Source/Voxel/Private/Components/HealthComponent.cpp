#include "Components/HealthComponent.h"

#include "Actors/EntityChunkActor.h"
#include "DataAssets/VoxelEntityDataAsset.h"

void UHealthComponent::InitializeComponent() {
  Super::InitializeComponent();

  EntityChunkActor->SetCacheEnabled(true);
}

void UHealthComponent::OnEntityAdded(const FIntVector& GlobalCoord,
                                     const FVoxelEntityData& NewEntityData) {
  Super::OnEntityAdded(GlobalCoord, NewEntityData);

  MaxHealth += NewEntityData.GetEntityDataAsset()->MaxDurability;
  CurrentHealth += NewEntityData.Durability;
}

void UHealthComponent::OnEntityRemoved(const FIntVector& GlobalCoord) {
  Super::OnEntityRemoved(GlobalCoord);

  if (const FVoxelEntityData* CacheData =
          EntityChunkActor->GetCache(GlobalCoord)) {
    MaxHealth -= CacheData->GetEntityDataAsset()->MaxDurability;
    CurrentHealth -= CacheData->Durability;
  }
}

void UHealthComponent::OnEntityModified(const FIntVector& GlobalCoord,
                                        const FVoxelEntityData& NewEntityData) {
  Super::OnEntityModified(GlobalCoord, NewEntityData);
  if (const FVoxelEntityData* CacheData =
          EntityChunkActor->GetCache(GlobalCoord)) {
    MaxHealth -= CacheData->GetEntityDataAsset()->MaxDurability;
    CurrentHealth -= CacheData->Durability;
  }

  MaxHealth += NewEntityData.GetEntityDataAsset()->MaxDurability;
  CurrentHealth += NewEntityData.Durability;
}