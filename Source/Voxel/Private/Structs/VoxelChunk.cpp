#include "Structs/VoxelChunk.h"
#include "DataAssets/VoxelBaseDataAsset.h"
#include "DataAssets/VoxelVoidDataAsset.h"

FVoxelChunk::FVoxelChunk(const int32 InChunkSize) {
  VoxelCount = InChunkSize * InChunkSize * InChunkSize;
  VoxelDataArray = new FVoxelBaseData *[VoxelCount];
  for (int32 i = 0; i < VoxelCount; ++i) {
    VoxelDataArray[i] = new FVoxelVoidData();
  }
}

FVoxelBaseData *FVoxelChunk::GetVoxelData(const int32 Index) const {
  if (Index >= 0 && Index < VoxelCount) {
    return VoxelDataArray[Index];
  }
  return new FVoxelVoidData();
}

void FVoxelChunk::SetVoxelData(const int32 Index,
                               FVoxelBaseData *VoxelBaseData) const {
  if (Index >= 0 && Index < VoxelCount && VoxelBaseData) {
    if (VoxelDataArray[Index] != VoxelBaseData) {
      delete VoxelDataArray[Index];
    }
    VoxelDataArray[Index] = VoxelBaseData;
  }
}
