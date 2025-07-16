#pragma once

#include "CoreMinimal.h"
#include "VoxelBaseDataAsset.h"
#include "VoxelSubstanceDataAsset.generated.h"

UCLASS(Abstract)
class VOXEL_API UVoxelSubstanceDataAsset : public UVoxelBaseDataAsset {
  GENERATED_BODY()

public:
  UVoxelSubstanceDataAsset();

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Voxel | Substance")
  FLinearColor VertexColor;
};

USTRUCT()
struct FVoxelSubstanceData : public FVoxelBaseData {
  GENERATED_BODY()

  FVoxelSubstanceData() = default;

  TObjectPtr<UVoxelSubstanceDataAsset> GetSubstanceDataAsset() const {
    return Cast<UVoxelSubstanceDataAsset>(DataAsset);
  }

  explicit FVoxelSubstanceData(
      const TObjectPtr<UVoxelSubstanceDataAsset> &InPtr)
      : FVoxelBaseData(InPtr) {}
};