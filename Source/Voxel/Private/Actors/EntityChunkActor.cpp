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

void AEntityChunkActor::BeginPlay() {
  Super::BeginPlay();

  if (const TObjectPtr<AVoxelWorldActor> VoxelWorldActor =
          Cast<AVoxelWorldActor>(UGameplayStatics::GetActorOfClass(
              GetWorld(), AVoxelWorldActor::StaticClass()))) {
    DataManager = VoxelWorldActor->GetComponentByClass<UDataManager>();
  }

  GetComponents<UEntityComponent>(Components);
}

void AEntityChunkActor::AddEntity(const FIntVector& VoxelCoord,
                                  const FVoxelEntityData& EntityData) {
  Entities.Add(VoxelCoord);
  UpdateLocation();

  for (const TObjectPtr<UEntityComponent>& Component : Components) {
    if (Component) {
      Component->OnEntityAdded(VoxelCoord, EntityData);
    }
  }
}

void AEntityChunkActor::RemoveEntity(const FIntVector& VoxelCoord,
                                     const FVoxelEntityData& EntityData) {
  Entities.Remove(VoxelCoord);
  UpdateLocation();

  for (const TObjectPtr<UEntityComponent>& Component : Components) {
    if (Component) {
      Component->OnEntityRemoved(VoxelCoord, EntityData);
    }
  }
}

bool AEntityChunkActor::IsEmpty() const {
  return Entities.IsEmpty();
}

void AEntityChunkActor::OnEntityModified(const FIntVector& VoxelCoord,
                                         const FVoxelEntityData& EntityData) {
  UpdateLocation();
  for (const TObjectPtr<UEntityComponent>& Component : Components) {
    if (Component) {
      Component->OnEntityModified(VoxelCoord, EntityData);
    }
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
