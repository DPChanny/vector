#include "DataAssets/VoxelBorderDataAsset.h"

FVoxelBaseData *UVoxelBorderDataAsset::ConstructVoxelData(
    const FVoxelBaseParams &Params) const {
  if (dynamic_cast<const FVoxelBorderParams *>(&Params)) {
    return new FVoxelBorderData(this);
  }
  return nullptr;
}
