#pragma once

#include "CoreMinimal.h"
#include "EntityChunk.generated.h"

class UEntityManager;

UCLASS()
class VOXEL_API UEntityChunk : public UObject {
  GENERATED_BODY()

public:
  virtual void Tick(float DeltaTime);

  void AddVoxel(const FIntVector &VoxelCoord);
  void RemoveVoxel(const FIntVector &VoxelCoord);

  bool IsEmpty() const;

private:
  UPROPERTY()
  TSet<FIntVector> ManagedVoxels;
};
