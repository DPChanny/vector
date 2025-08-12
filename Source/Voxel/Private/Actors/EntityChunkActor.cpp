#include "Actors/EntityChunkActor.h"
#include "Actors/VoxelWorldActor.h"
#include "Components/EntityComponent.h"
#include "DataAssets/VoxelEntityDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Managers/DataManager.h"

bool AEntityChunkActor::IsChunkableWith(const FVoxelEntityData& Other) const {
  if (Entities.IsEmpty()) {
    return false;
  }
  return dynamic_cast<const FVoxelEntityData*>(
             DataManager->GetVoxelData(*Entities.begin()))
      ->IsChunkableWith(&Other);
}

FVoxelEntityData* AEntityChunkActor::GetCache(const FIntVector& GlobalCoord) {
  if (FVoxelEntityData** CacheData = Cache.Find(GlobalCoord)) {
    return *CacheData;
  }
  return nullptr;
}

void AEntityChunkActor::BeginPlay() {
  Super::BeginPlay();

  if (const TObjectPtr<AVoxelWorldActor> VoxelWorldActor =
          Cast<AVoxelWorldActor>(UGameplayStatics::GetActorOfClass(
              GetWorld(), AVoxelWorldActor::StaticClass()))) {
    DataManager = VoxelWorldActor->GetComponentByClass<UDataManager>();
  }

  GetComponents<UEntityComponent>(Components);
}

void AEntityChunkActor::AddEntity(const FIntVector& GlobalCoord,
                                  const FVoxelEntityData& NewEntityData) {
  Entities.Add(GlobalCoord);
  UpdateLocation();

  for (const TObjectPtr<UEntityComponent>& Component : Components) {
    if (Component) {
      Component->OnEntityAdded(GlobalCoord, NewEntityData);
    }
  }

  if (bCacheEnabled) {
    Cache.Add(GlobalCoord,
              dynamic_cast<FVoxelEntityData*>(NewEntityData.Clone()));
  }
}

void AEntityChunkActor::RemoveEntity(const FIntVector& GlobalCoord) {
  Entities.Remove(GlobalCoord);
  UpdateLocation();

  for (const TObjectPtr<UEntityComponent>& Component : Components) {
    if (Component) {
      Component->OnEntityRemoved(GlobalCoord);
    }
  }

  if (bCacheEnabled) {
    Cache.Remove(GlobalCoord);
  }
}

bool AEntityChunkActor::IsEmpty() const {
  return Entities.IsEmpty();
}

void AEntityChunkActor::OnEntityModified(
    const FIntVector& GlobalCoord, const FVoxelEntityData& NewEntityData) {
  UpdateLocation();

  for (const TObjectPtr<UEntityComponent>& Component : Components) {
    if (Component) {
      Component->OnEntityModified(GlobalCoord, NewEntityData);
    }
  }

  if (bCacheEnabled) {
    Cache.Add(GlobalCoord,
              dynamic_cast<FVoxelEntityData*>(NewEntityData.Clone()));
  }
}

void AEntityChunkActor::UpdateLocation() {
  FVector Location = FVector::ZeroVector;
  for (const FIntVector& VoxelCoord : Entities) {
    Location += FVector(DataManager->GlobalToWorldCoord(VoxelCoord));
  }
  Location /= Entities.Num();
  SetActorLocation(Location);
}
