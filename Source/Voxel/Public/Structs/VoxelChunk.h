#pragma once

#include "Actors/VoxelChunkActor.h"
#include "CoreMinimal.h"
#include "Voxel.h"

// clang-format off
#include "VoxelChunk.generated.h"
// clang-format on

class AVoxelChunkActor;

USTRUCT()
struct FVoxelChunk {
  GENERATED_BODY()

  TArray<FVoxel> Voxels;
  UPROPERTY()
  TObjectPtr<AVoxelChunkActor> VoxelChunkActor;

  void Initialize(const int32 ChunkVolume) {
    Voxels.Init(FVoxel(), ChunkVolume);
    VoxelChunkActor = nullptr;
  }

  FVoxel GetVoxel(const int32 Index) const {
    return Voxels.IsValidIndex(Index) ? Voxels[Index] : FVoxel();
  }

  void SetVoxel(const int32 Index, const FVoxel &Voxel) {
    if (Voxels.IsValidIndex(Index)) {
      Voxels[Index] = Voxel;
    }
  }
};
