#pragma once

#include "Components/EntityComponent.h"
#include "CoreMinimal.h"
#include "SurfaceComponent.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))

class VOXEL_API USurfaceComponent : public UEntityComponent {
  GENERATED_BODY()

 public:
  USurfaceComponent();

  FIntVector GetClosestSurfaceVoxel(const FVector& TargetCoord) const;

  virtual void OnEntityAdded(const FIntVector& GlobalCoord,
                             const FVoxelEntityData& NewEntityData) override;
  virtual void OnEntityRemoved(const FIntVector& GlobalCoord) override;
  virtual void OnEntityModified(const FIntVector& GlobalCoord,
                                const FVoxelEntityData& NewEntityData) override;

 private:
  TSet<FIntVector> SurfaceVoxels;

  bool IsSurfaceVoxel(const FIntVector& GlobalCoord) const;

  void UpdateSurfaceAroundVoxel(const FIntVector& GlobalCoord);

  static const TArray<FIntVector> NeighborOffsets;
};
