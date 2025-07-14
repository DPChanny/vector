#pragma once

#include "CoreMinimal.h"
#include "VoxelBuild.generated.h"

class UVoxelData;
class UVoxelMesh;

UCLASS()
class VOXEL_API UVoxelBuild : public UObject {
  GENERATED_BODY()

public:
  void Initialize();

  void DamageVoxel(const FVector &Center, float Radius,
                   float DamageAmount) const;
  void ConstructVoxel(const FVector &Center, float Radius,
                      float ConstructionAmount, int32 VoxelIDToConstruct) const;

  void GetGlobalCoordsInRadius(const FVector &Center, float Radius,
                               TSet<FIntVector> &FoundGlobalCoords) const;

private:
  void
  ProcessVoxel(const FVector &Center, float Radius,
               const TFunction<void(const FIntVector &)> &VoxelModifier) const;
  bool IsSurfaceVoxel(const FIntVector &VoxelCoord) const;

  UPROPERTY()
  TObjectPtr<UVoxelData> VoxelData;

  UPROPERTY()
  TObjectPtr<UVoxelMesh> VoxelMesh;
};
