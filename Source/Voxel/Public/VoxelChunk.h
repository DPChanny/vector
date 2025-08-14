#pragma once

#include "CoreMinimal.h"

#include "VoxelChunk.generated.h"

struct FVoxelBaseData;
struct FVoxelBaseParams;
class UVoxelBaseDataAsset;
class AVoxelChunkActor;

USTRUCT()

struct VOXEL_API FVoxelChunk {
  GENERATED_BODY()

  FVoxelChunk() = default;

  explicit FVoxelChunk(const int32 InChunkSize,
                       const TObjectPtr<UVoxelBaseDataAsset>& InVoxelDataAsset,
                       const FVoxelBaseParams& InVoxelParams);

  FVoxelBaseData* GetVoxelData(int32 Index) const;
  void SetVoxelData(int32 Index, FVoxelBaseData* VoxelBaseData) const;

  UPROPERTY()
  TObjectPtr<AVoxelChunkActor> VoxelChunkActor;

 private:
  int32 VoxelCount;
  FVoxelBaseData** VoxelDataArray;
};
