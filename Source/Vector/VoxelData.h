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
      const TObjectPtr<UVoxelBorderDataAsset> &InVoxelBorderDataAsset);

  void LoadChunk(const FIntVector &ChunkCoord);
  void UnloadChunk(const FIntVector &ChunkCoord);
  inline bool IsChunk(const FIntVector &ChunkCoord) const;
  FChunk *GetChunk(const FIntVector &ChunkCoord);

  FVoxel GetVoxel(const FIntVector &GlobalCoord) const;
  void SetVoxel(const FIntVector &GlobalCoord, const FVoxel &Voxel,
                bool bAutoDebug = true);

  int32 GetVoxelID(const FIntVector &GlobalCoord) const;
  int32 GetVoxelID(const TObjectPtr<UVoxelBaseDataAsset> &VoxelDataAsset);
  void SetVoxelID(const FIntVector &GlobalCoord, int32 NewVoxelID);

  float GetDurability(const FIntVector &GlobalCoord) const;
  void SetDurability(const FIntVector &GlobalVoxelCoord, float NewDurability);

  float GetDensity(const FIntVector &GlobalCoord) const;

  TObjectPtr<UVoxelBaseDataAsset> GetVoxelDataAsset(int32 VoxelID) const;
  TObjectPtr<UVoxelBaseDataAsset>
  GetVoxelDataAsset(FIntVector GlobalCoord) const;

  int32 GetChunkSize() const { return ChunkSize; }
  int32 GetVoxelSize() const { return VoxelSize; }

  inline FIntVector GlobalToChunkCoord(const FIntVector &GlobalCoord) const;
  inline FIntVector ChunkToGlobalCoord(const FIntVector &ChunkCoord) const;
  inline FIntVector GlobalToLocalCoord(const FIntVector &GlobalCoord) const;
  inline FIntVector LocalToGlobalCoord(const FIntVector &LocalCoord,
                                       const FIntVector &ChunkCoord) const;
  inline FIntVector WorldToGlobalCoord(const FVector &WorldCoord) const;
  inline FVector GlobalToWorldCoord(const FIntVector &GlobalCoord) const;

  inline int32 LocalCoordToIndex(const FIntVector &LocalCoord) const;

private:
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
