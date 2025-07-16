#pragma once

#include "CoreMinimal.h"
#include "DebugManager.generated.h"

class AVoxelDebugActor;
class UDataManager;

UCLASS()
class VOXEL_API UDebugManager : public UObject {
  GENERATED_BODY()

public:
  void Initialize(const TSubclassOf<AVoxelDebugActor> &InDebugActorClass);

  void SetDebugVoxels(const TSet<FIntVector> &NewDebugVoxels,
                      const FColor &Color = FColor::Green);
  void SetDebugVoxel(const FIntVector &NewDebugVoxel,
                     const FColor &Color = FColor::Green);
  void FlushDebugVoxelBuffer();

private:
  void AddDebugVoxel(const FIntVector &GlobalCoord);
  void RemoveDebugVoxel(const FIntVector &VoxelCoord);

  UPROPERTY()
  TSubclassOf<AVoxelDebugActor> DebugActorClass;

  UPROPERTY()
  TObjectPtr<UDataManager> DataManager;

  TMap<FIntVector, TObjectPtr<AVoxelDebugActor>> DebugVoxels;
  TMap<FIntVector, FColor> DebugVoxelsBuffer;
};
