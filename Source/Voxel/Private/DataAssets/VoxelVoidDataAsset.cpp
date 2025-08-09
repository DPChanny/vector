#include "DataAssets/VoxelVoidDataAsset.h"

UVoxelVoidDataAsset::UVoxelVoidDataAsset() {
  VoxelName = "Void";
  BaseDensity = -1.0f;
}

FVoxelBaseData *
UVoxelVoidDataAsset::ConstructVoxelData(const FVoxelBaseParams &Params) const {
  if (dynamic_cast<const FVoxelVoidParams *>(&Params)) {
    return new FVoxelVoidData(this);
  }
  return nullptr;
}
