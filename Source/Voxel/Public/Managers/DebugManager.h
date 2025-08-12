#pragma once

#include "CoreMinimal.h"
#include "DebugManager.generated.h"

class AVoxelDebugActor;
class UDataManager;

UCLASS()

class VOXEL_API UDebugManager : public UActorComponent {
  GENERATED_BODY()

 public:
  void SetDebugVoxels(const TSet<FIntVector>& NewDebugVoxels,
                      const FColor& Color = FColor::Green);
  void SetDebugVoxel(const FIntVector& NewDebugVoxel,
                     const FColor& Color = FColor::Green);
  void FlushDebugVoxelBuffer();

 private:
  virtual void InitializeComponent() override;

  void AddDebugVoxel(const FIntVector& GlobalCoord);
  void RemoveDebugVoxel(const FIntVector& VoxelCoord);

  UPROPERTY(EditDefaultsOnly)
  TSubclassOf<AVoxelDebugActor> VoxelDebugActorClass;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UDataManager> DataManager;

  TMap<FIntVector, TObjectPtr<AVoxelDebugActor>> DebugVoxels;
  TMap<FIntVector, FColor> DebugVoxelsBuffer;
};
