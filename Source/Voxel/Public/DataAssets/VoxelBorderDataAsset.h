#pragma once

#include "CoreMinimal.h"
#include "VoxelSubstanceDataAsset.h"

#include "VoxelBorderDataAsset.generated.h"

UCLASS()

class VOXEL_API UVoxelBorderDataAsset : public UVoxelSubstanceDataAsset {
  GENERATED_BODY()

 public:
  virtual FVoxelBaseData* ConstructVoxelData(
      const FVoxelBaseParams& Params) const override;
};

USTRUCT()

struct FVoxelBorderData : public FVoxelSubstanceData {
  GENERATED_BODY()

  FVoxelBorderData() = default;

  static bool IsBorder(const FVoxelBaseData* VoxelBaseData) {
    return dynamic_cast<const FVoxelBorderData*>(VoxelBaseData) != nullptr;
  }

  TObjectPtr<const UVoxelBorderDataAsset> GetVoidDataAsset() const {
    return Cast<const UVoxelBorderDataAsset>(DataAsset);
  }

  explicit FVoxelBorderData(
      const TObjectPtr<const UVoxelBorderDataAsset>& InPtr)
      : FVoxelSubstanceData(InPtr) {}

  virtual FVoxelBaseData* Clone() const override {
    return new FVoxelBorderData(*this);
  }
};

struct FVoxelBorderParams final : FVoxelSubstanceParams {
  virtual FVoxelBaseParams* Clone() const override {
    return new FVoxelBorderParams(*this);
  }
};
