#include "DataAssets/VoxelBlockDataAsset.h"

FVoxelBaseData *
UVoxelBlockDataAsset::ConstructVoxelData(const FVoxelBaseParams &Params) const {
  if (const FVoxelBlockParams *BlockParams =
          dynamic_cast<const FVoxelBlockParams *>(&Params)) {
    return new FVoxelBlockData(this, BlockParams->Durability);
  }
  return nullptr;
}
