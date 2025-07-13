#pragma once

#include "CoreMinimal.h"
#include "VoxelChunk.h"

// clang-format off
#include "Voxel.generated.h"
// clang-format on

class AVoxelChunk;

USTRUCT(BlueprintType)
struct FVoxel {
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
  int32 ID;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
  float Durability;

  FVoxel() : ID(0), Durability(0.f) {}
  FVoxel(const int32 InID, const float InDurability)
      : ID(InID), Durability(InDurability) {}
};

USTRUCT()
struct FChunk {
  GENERATED_BODY()

  TArray<FVoxel> Voxels;
  UPROPERTY()
  TObjectPtr<AVoxelChunk> VoxelChunk;

  void Initialize(const int32 ChunkVolume) {
    Voxels.Init(FVoxel(), ChunkVolume);
    VoxelChunk = nullptr;
  }

  void UpdateMesh() const {
    if (VoxelChunk) {
      VoxelChunk->UpdateMesh();
    }
  }

  const FVoxel &GetVoxel(const int32 Index) const {
    return Voxels.IsValidIndex(Index) ? Voxels[Index] : FVoxel();
  }

  void SetVoxel(const int32 Index, const FVoxel &Voxel) {
    if (Voxels.IsValidIndex(Index)) {
      Voxels[Index] = Voxel;
    }
  }
};
