#pragma once

#include "CoreMinimal.h"
#include "VoxelSubstanceDataAsset.h"

// clang-format off
#include "VoxelBorderDataAsset.generated.h"
// clang-format on

UCLASS()
class VOXEL_API UVoxelBorderDataAsset : public UVoxelSubstanceDataAsset {
  GENERATED_BODY()

public:
  virtual FVoxelBaseData *
  ConstructVoxelData(const FVoxelBaseParams &Params) const override;
};

USTRUCT()
struct FVoxelBorderData : public FVoxelSubstanceData {
  GENERATED_BODY()

  FVoxelBorderData() = default;

  static bool IsBorder(const FVoxelBaseData *VoxelBaseData) {
    return dynamic_cast<const FVoxelBorderData *>(VoxelBaseData) != nullptr;
  }

  TObjectPtr<const UVoxelBorderDataAsset> GetVoidDataAsset() const {
    return Cast<const UVoxelBorderDataAsset>(DataAsset);
  }

  explicit FVoxelBorderData(
      const TObjectPtr<const UVoxelBorderDataAsset> &InPtr)
      : FVoxelSubstanceData(InPtr) {}
};

struct FVoxelBorderParams : FVoxelSubstanceParams {
  virtual FVoxelBaseParams *Clone() const override {
    return new FVoxelBorderParams(*this);
  }
};
