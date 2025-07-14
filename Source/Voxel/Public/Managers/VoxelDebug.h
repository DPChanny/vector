#pragma once

#include "CoreMinimal.h"
#include "VoxelDebug.generated.h"

class AVoxelDebugActor;
class UVoxelData;

UCLASS()
class VOXEL_API UVoxelDebug : public UObject {
  GENERATED_BODY()

public:
  void Initialize(const TSubclassOf<AVoxelDebugActor> &InDebugActorClass);

  void SetDebugVoxels(const TSet<FIntVector> &NewDebugVoxels);
  void SetDebugVoxel(const FIntVector &NewDebugVoxel);
  void FlushDebugVoxelBuffer();

private:
  void AddDebugVoxel(const FIntVector &GlobalCoord);
  void RemoveDebugVoxel(const FIntVector &VoxelCoord);

  UPROPERTY()
  TSubclassOf<AVoxelDebugActor> DebugActorClass;

  UPROPERTY()
  TObjectPtr<UVoxelData> VoxelData;

  TMap<FIntVector, TObjectPtr<AVoxelDebugActor>> DebugVoxels;
  TSet<FIntVector> DebugVoxelsBuffer;
};
