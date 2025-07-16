#pragma once

#include "CoreMinimal.h"

// clang-format off
#include "BuildManager.generated.h"
// clang-format on

class UVoxelBlockDataAsset;
class UDataManager;
class UMeshManager;
class UDebugManager;

const TArray NeighborOffsets = {FIntVector(1, 0, 0), FIntVector(-1, 0, 0),
                                FIntVector(0, 1, 0), FIntVector(0, -1, 0),
                                FIntVector(0, 0, 1), FIntVector(0, 0, -1)};

UCLASS()
class VOXEL_API UBuildManager : public UObject {
  GENERATED_BODY()

public:
  void Initialize();

  void DamageVoxelsInRadius(const FIntVector &CenterGlobalCoord, float Radius,
                            float DamageAmount) const;
  void ConstructVoxelsInRadius(
      const FIntVector &CenterGlobalCoord, float Radius,
      float ConstructionAmount,
      const TObjectPtr<UVoxelBlockDataAsset> &NewVoxelBlockDataAsset) const;

  void GetGlobalCoordsInRadius(const FIntVector &CenterGlobalCoord,
                               float Radius,
                               TSet<FIntVector> &FoundGlobalCoords) const;

private:
  void ProcessVoxelsInRadius(
      const FIntVector &CenterGlobalCoord, float Radius,
      const TFunction<void(const FIntVector &)> &VoxelModifier) const;
  bool IsSurfaceVoxel(const FIntVector &VoxelCoord) const;

  UPROPERTY()
  TObjectPtr<UDataManager> DataManager;

  UPROPERTY()
  TObjectPtr<UMeshManager> MeshManager;
};
