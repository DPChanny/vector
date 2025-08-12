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

  TObjectPtr<const UVoxelSubstanceDataAsset> GetSubstanceDataAsset() const {
    return Cast<const UVoxelSubstanceDataAsset>(DataAsset);
  }

  explicit FVoxelSubstanceData(
      const TObjectPtr<const UVoxelSubstanceDataAsset>& InDataAsset)
      : FVoxelBaseData(InDataAsset) {}

  virtual FVoxelBaseData* Clone() const override {
    return new FVoxelSubstanceData(*this);
  };
};

struct FVoxelSubstanceParams : FVoxelBaseParams {
  virtual FVoxelBaseParams* Clone() const override {
    return new FVoxelSubstanceParams(*this);
  }
};
