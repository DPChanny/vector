#pragma once

#include "CoreMinimal.h"
#include "EntityComponent.generated.h"

struct FVoxelEntityData;

UCLASS(Abstract)
class VOXEL_API UEntityComponent : public UObject {
  GENERATED_BODY()

public:
  explicit UEntityComponent();

  virtual void OnEntityAdded(const FIntVector &VoxelCoord,
                             const FVoxelEntityData &Data) {}
  virtual void OnEntityRemoved(const FIntVector &VoxelCoord,
                               const FVoxelEntityData &Data) {}
  virtual void OnEntityDataModified(const FIntVector &VoxelCoord,
                                    const FVoxelEntityData &Data) {}
  virtual void Tick(float DeltaTime) {}

protected:
  UPROPERTY()
  TObjectPtr<class UEntityChunk> OwnerChunk;
};
