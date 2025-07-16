#pragma once

#include "CoreMinimal.h"
#include "VoxelSubstanceDataAsset.h"

// clang-format off
#include "VoxelBorderDataAsset.generated.h"
// clang-format on

UCLASS()
class VOXEL_API UVoxelBorderDataAsset : public UVoxelSubstanceDataAsset {
  GENERATED_BODY()
};

USTRUCT()
struct FVoxelBorderData : public FVoxelSubstanceData {
  GENERATED_BODY()

  FVoxelBorderData() = default;

  static bool IsBorder(const FVoxelBaseData *VoxelBaseData) {
    return dynamic_cast<const FVoxelBorderData *>(VoxelBaseData) != nullptr;
  }

  TObjectPtr<UVoxelBorderDataAsset> GetVoidDataAsset() const {
    return Cast<UVoxelBorderDataAsset>(DataAsset);
  }

  explicit FVoxelBorderData(const TObjectPtr<UVoxelBorderDataAsset> &InPtr)
      : FVoxelSubstanceData(InPtr) {}
};
