#pragma once

#include "CoreMinimal.h"
#include "EntityChunk.generated.h"

class UEntityManager;

UCLASS(Abstract)
class VOXEL_API UEntityChunk : public UObject {
  GENERATED_BODY()

public:
  virtual void Tick(float DeltaTime);

  void AddEntity(const FIntVector &VoxelCoord);
  void RemoveEntity(const FIntVector &VoxelCoord);

  bool IsEmpty() const;

  virtual void OnEntityDataModified(const FIntVector &VoxelCoord);

  const TSet<FIntVector> &GetManagedVoxels() const { return ManagedVoxels; }

private:
  UPROPERTY()
  TSet<FIntVector> ManagedVoxels;
};
