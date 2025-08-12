#include "VoxelChunk.h"

#include "DataAssets/VoxelBaseDataAsset.h"

FVoxelChunk::FVoxelChunk(
    const int32 InChunkSize,
    const TObjectPtr<UVoxelBaseDataAsset>& InVoxelDataAsset,
    const FVoxelBaseParams& InVoxelParams) {
  VoxelCount = InChunkSize * InChunkSize * InChunkSize;
  VoxelDataArray = new FVoxelBaseData*[VoxelCount];
  for (int32 i = 0; i < VoxelCount; ++i) {
    VoxelDataArray[i] = InVoxelDataAsset->ConstructVoxelData(InVoxelParams);
  }
}

FVoxelBaseData* FVoxelChunk::GetVoxelData(const int32 Index) const {
  if (Index >= 0 && Index < VoxelCount) {
    return VoxelDataArray[Index];
  }
  return nullptr;
}

void FVoxelChunk::SetVoxelData(const int32 Index,
                               FVoxelBaseData* VoxelBaseData) const {
  if (Index >= 0 && Index < VoxelCount && VoxelBaseData) {
    VoxelDataArray[Index] = VoxelBaseData;
  }
}
