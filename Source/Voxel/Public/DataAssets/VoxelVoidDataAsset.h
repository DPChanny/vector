#pragma once

#include "CoreMinimal.h"
#include "VoxelBaseDataAsset.h"
#include "VoxelVoidDataAsset.generated.h"

UCLASS()
class VOXEL_API UVoxelVoidDataAsset : public UVoxelBaseDataAsset {
  GENERATED_BODY()

public:
  UVoxelVoidDataAsset();
};

USTRUCT()
struct FVoxelVoidData : public FVoxelBaseData {
  GENERATED_BODY()

  static bool IsVoid(const FVoxelBaseData *VoxelBaseData) {
    return dynamic_cast<const FVoxelVoidData *>(VoxelBaseData) != nullptr;
  }

  TObjectPtr<UVoxelVoidDataAsset> GetVoidDataAsset() const {
    return Cast<UVoxelVoidDataAsset>(DataAsset);
  }

  explicit FVoxelVoidData()
      : FVoxelBaseData(UVoxelVoidDataAsset::StaticClass()
                           ->GetDefaultObject<UVoxelVoidDataAsset>()) {}
};
