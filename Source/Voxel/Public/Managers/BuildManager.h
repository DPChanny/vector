#pragma once

#include "CoreMinimal.h"
#include "DataAssets/VoxelBaseDataAsset.h"
#include "DataAssets/VoxelBlockDataAsset.h"

// clang-format off
#include "BuildManager.generated.h"
// clang-format on

class UVoxelBlockDataAsset;
class UDataManager;
class UMeshManager;
class UDebugManager;

UCLASS()

class VOXEL_API UBuildManager : public UActorComponent {
  GENERATED_BODY()

  const FIntVector NeighborOffsets[6] = {
      FIntVector(1, 0, 0),  FIntVector(-1, 0, 0), FIntVector(0, 1, 0),
      FIntVector(0, -1, 0), FIntVector(0, 0, 1),  FIntVector(0, 0, -1)};

 public:
  void DamageBlocksInRadius(const FIntVector& CenterGlobalCoord, float Radius,
                            float DamageAmount) const;
  void ConstructBlocksInRadius(
      const FIntVector& CenterGlobalCoord, float Radius,
      float ConstructionAmount,
      const TObjectPtr<const UVoxelBlockDataAsset>& NewVoxelBlockDataAsset,
      const FVoxelBlockParams& VoxelParams) const;

  void GetGlobalCoordsInRadius(const FIntVector& CenterGlobalCoord,
                               float Radius,
                               TSet<FIntVector>& FoundGlobalCoords) const;

 private:
  virtual void InitializeComponent() override;

  void ProcessVoxelsInRadius(
      const FIntVector& CenterGlobalCoord, float Radius,
      const TFunction<void(const FIntVector&)>& VoxelModifier) const;
  bool IsSurfaceVoxel(const FIntVector& VoxelCoord) const;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UDataManager> DataManager;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UMeshManager> MeshManager;
};
