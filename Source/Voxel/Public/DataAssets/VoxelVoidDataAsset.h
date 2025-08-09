#pragma once

#include "CoreMinimal.h"
#include "VoxelBaseDataAsset.h"
#include "VoxelVoidDataAsset.generated.h"

UCLASS()
class VOXEL_API UVoxelVoidDataAsset : public UVoxelBaseDataAsset {
  GENERATED_BODY()

public:
  UVoxelVoidDataAsset();

  virtual FVoxelBaseData *
  ConstructVoxelData(const FVoxelBaseParams &Params) const override;
};

USTRUCT()
struct FVoxelVoidData : public FVoxelBaseData {
  GENERATED_BODY()

  explicit FVoxelVoidData()
      : FVoxelBaseData(UVoxelVoidDataAsset::StaticClass()
                           ->GetDefaultObject<UVoxelVoidDataAsset>()) {}

  static bool IsVoid(const FVoxelBaseData *VoxelBaseData) {
    return dynamic_cast<const FVoxelVoidData *>(VoxelBaseData) != nullptr;
  }

  TObjectPtr<const UVoxelVoidDataAsset> GetVoidDataAsset() const {
    return Cast<const UVoxelVoidDataAsset>(DataAsset);
  }

  explicit FVoxelVoidData(
      const TObjectPtr<const UVoxelVoidDataAsset> &InDataAsset)
      : FVoxelBaseData(InDataAsset) {}
};

struct FVoxelVoidParams final : FVoxelBaseParams {
  virtual FVoxelBaseParams *Clone() const override {
    return new FVoxelVoidParams(*this);
  }
};
