#pragma once

#include "CoreMinimal.h"
#include "Voxel.h"
#include "VoxelBaseDataAsset.h"
#include "VoxelBlockDataAsset.h"
#include "VoxelBorderDataAsset.h"
#include "VoxelVoidDataAsset.h"

// clang-format off
#include "VoxelData.generated.h"
// clang-format on

class UVoxelBaseDataAsset;
class AVoxelChunk;
class UVoxelDebug;

UCLASS()
class VECTOR_API UVoxelData : public UObject {
  GENERATED_BODY()

public:
  static int32 GetVoidID() { return 0; }
  static int32 GetBorderID() { return 1; }
  static int32 GetDefaultBlockID() { return 2; }

  static float GetSurfaceLevel() { return 0.f; }

  void Initialize(
      const FIntVector &InWorldSizeInChunks, int32 InChunkSize,
      int32 InVoxelSize, const TSubclassOf<AVoxelChunk> &InVoxelChunk,
      const TArray<TObjectPtr<UVoxelBlockDataAsset>> &InVoxelBlockDataAssets,
      const TObjectPtr<UVoxelVoidDataAsset> &InVoxelVoidDataAsset,
      const TObjectPtr<UVoxelBorderDataAsset> &InVoxelBorderDataAsset,
      UVoxelDebug *InVoxelDebug);

  void LoadChunk(const FIntVector &ChunkCoord);
  void UnloadChunk(const FIntVector &ChunkCoord);

  FVoxel GetVoxel(const FIntVector &GlobalVoxelCoord) const;
  void SetVoxel(const FIntVector &GlobalVoxelCoord, const FVoxel &Voxel,
                bool bAutoDebug = true);

  int32 GetVoxelID(const FIntVector &GlobalVoxelCoord) const;
  int32 GetVoxelID(const TObjectPtr<UVoxelBaseDataAsset> &VoxelDataAsset);

  void SetVoxelID(const FIntVector &GlobalVoxelCoord, int32 NewVoxelID);

  float GetDurability(const FIntVector &GlobalVoxelCoord) const;
  void SetDurability(const FIntVector &GlobalVoxelCoord, float NewDurability);

  float GetDensity(const FIntVector &GlobalVoxelCoord) const;
  UVoxelBaseDataAsset *GetVoxelDataAsset(int32 VoxelID) const;

  bool IsChunkLoaded(const FIntVector &ChunkCoord) const;
  FChunk *GetChunk(const FIntVector &ChunkCoord);

  int32 GetChunkSize() const { return ChunkSize; }
  int32 GetVoxelSize() const { return VoxelSize; }

private:
  FIntVector GlobalToChunkCoord(const FIntVector &GlobalVoxelCoord) const;
  FIntVector GlobalToLocalCoord(const FIntVector &GlobalVoxelCoord) const;
  int32 LocalCoordToIndex(const FIntVector &LocalVoxelCoord) const;

  TMap<FIntVector, FChunk> Chunks;
  TMap<int32, TObjectPtr<UVoxelBaseDataAsset>> VoxelDataAssets;

  UPROPERTY()
  TSubclassOf<AVoxelChunk> VoxelChunk;
  int32 VoxelSize;

  FIntVector WorldSizeInChunks;
  int32 ChunkSize;
  int32 ChunkVolume;

  UPROPERTY()
  TObjectPtr<UVoxelDebug> VoxelDebug;
};
