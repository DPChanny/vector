﻿#pragma once

#include "CoreMinimal.h"

// clang-format off
#include "VoxelChunk.generated.h"
// clang-format on

struct FVoxelBaseData;
class AVoxelChunkActor;

USTRUCT()
struct VOXEL_API FVoxelChunk {
  GENERATED_BODY()

  FVoxelChunk() = default;

  explicit FVoxelChunk(const int32 InChunkSize);

  FVoxelBaseData *GetVoxelData(int32 Index) const;
  void SetVoxelData(int32 Index, FVoxelBaseData *VoxelBaseData) const;

  UPROPERTY()
  TObjectPtr<AVoxelChunkActor> VoxelChunkActor;

private:
  FVoxelBaseData **VoxelDataArray;
  int32 VoxelCount;
};
