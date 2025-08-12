#pragma once

#include "CoreMinimal.h"
#include "EntityComponent.generated.h"

struct FVoxelEntityData;

UCLASS(Abstract, meta = (BlueprintSpawnableComponent), Blueprintable)

class VOXEL_API UEntityComponent : public UActorComponent {
  GENERATED_BODY()

 public:
  virtual void OnEntityAdded(const FIntVector& VoxelCoord,
                             const FVoxelEntityData& Data) {}

  virtual void OnEntityRemoved(const FIntVector& VoxelCoord,
                               const FVoxelEntityData& Data) {}

  virtual void OnEntityModified(const FIntVector& VoxelCoord,
                                const FVoxelEntityData& Data) {}
};
