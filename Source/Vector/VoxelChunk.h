#pragma once

#include "CoreMinimal.h"
#include "Voxel.h"
#include "VoxelChunkActor.h"

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

  void Update() const {
    if (VoxelChunkActor) {
      VoxelChunkActor->UpdateMesh();
    }
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
