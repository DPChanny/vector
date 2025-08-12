#include "Actors/EntityChunkActor.h"
#include "Actors/VoxelWorldActor.h"
#include "Components/EntityComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Managers/DataManager.h"

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
  ManagedVoxels.Add(VoxelCoord);
  UpdateLocation();

  for (const TObjectPtr<UEntityComponent>& Component : Components) {
    if (Component) {
      Component->OnEntityAdded(VoxelCoord, EntityData);
    }
  }
}

void AEntityChunkActor::RemoveEntity(const FIntVector& VoxelCoord,
                                     const FVoxelEntityData& EntityData) {
  ManagedVoxels.Remove(VoxelCoord);
  UpdateLocation();

  for (const TObjectPtr<UEntityComponent>& Component : Components) {
    if (Component) {
      Component->OnEntityRemoved(VoxelCoord, EntityData);
    }
  }
}

bool AEntityChunkActor::IsEmpty() const {
  return ManagedVoxels.IsEmpty();
}

void AEntityChunkActor::OnEntityDataModified(
    const FIntVector& VoxelCoord, const FVoxelEntityData& EntityData) {
  UpdateLocation();
  for (const TObjectPtr<UEntityComponent>& Component : Components) {
    if (Component) {
      Component->OnEntityDataModified(VoxelCoord, EntityData);
    }
  }
}

void AEntityChunkActor::UpdateLocation() {
  FVector Location = FVector::ZeroVector;
  for (const FIntVector& VoxelCoord : ManagedVoxels) {
    Location += FVector(DataManager->GlobalToWorldCoord(VoxelCoord));
  }
  Location /= ManagedVoxels.Num();
  SetActorLocation(Location);
}
