#include "DataAssets/VoxelEntityDataAsset.h"

FVoxelBaseData *UVoxelEntityDataAsset::ConstructVoxelData(
    const FVoxelBaseParams &Params) const {
  if (const FVoxelEntityParams *EntityParams =
          dynamic_cast<const FVoxelEntityParams *>(&Params)) {
    return new FVoxelEntityData(this, EntityParams->Durability,
                                EntityParams->TeamID);
  }
  return nullptr;
}
