#pragma once

#include "CoreMinimal.h"
#include "VoxelChunk.h"

// clang-format off
#include "VoxelData.generated.h"
// clang-format on

class UVoxelMesh;
class UVoxelBorderDataAsset;
class UVoxelVoidDataAsset;
class UVoxelBlockDataAsset;
class UVoxelBaseDataAsset;
class AVoxelChunkActor;
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
      int32 InVoxelSize, const TSubclassOf<AVoxelChunkActor> &InVoxelChunk,
      const TArray<TObjectPtr<UVoxelBlockDataAsset>> &InVoxelBlockDataAssets,
      const TObjectPtr<UVoxelVoidDataAsset> &InVoxelVoidDataAsset,
      const TObjectPtr<UVoxelBorderDataAsset> &InVoxelBorderDataAsset);

  void LoadChunk(const FIntVector &ChunkCoord);
  void UnloadChunk(const FIntVector &ChunkCoord);
  inline bool IsChunk(const FIntVector &ChunkCoord) const;
  FVoxelChunk *GetChunk(const FIntVector &ChunkCoord);

  FVoxel GetVoxel(const FIntVector &GlobalCoord) const;
  void SetVoxel(const FIntVector &GlobalCoord, const FVoxel &Voxel,
                bool bAutoDebug = true);

  int32 GetVoxelID(const FIntVector &GlobalCoord) const;
  int32 GetVoxelID(const TObjectPtr<UVoxelBaseDataAsset> &VoxelDataAsset);
  void SetVoxelID(const FIntVector &GlobalCoord, int32 NewVoxelID);

  float GetDurability(const FIntVector &GlobalCoord) const;
  void SetDurability(const FIntVector &GlobalVoxelCoord, float NewDurability);

  float GetDensity(const FIntVector &GlobalCoord) const;

  template <typename T>
  TObjectPtr<T> GetVoxelDataAsset(const int32 VoxelID) const {
    if (const TObjectPtr<UVoxelBaseDataAsset> FoundData =
            VoxelDataAssets.FindRef(VoxelID)) {
      return Cast<T>(FoundData);
    }
    return Cast<T>(VoxelDataAssets.FindRef(GetVoidID()));
  }

  template <typename T>
  TObjectPtr<T> GetVoxelDataAsset(const FIntVector GlobalCoord) const {
    return GetVoxelDataAsset<T>(GetVoxelID(GlobalCoord));
  }

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
  TMap<FIntVector, FVoxelChunk> Chunks;
  TMap<int32, TObjectPtr<UVoxelBaseDataAsset>> VoxelDataAssets;

  UPROPERTY()
  TSubclassOf<AVoxelChunkActor> VoxelChunk;
  int32 VoxelSize;

  FIntVector WorldSizeInChunks;
  int32 ChunkSize;
  int32 ChunkVolume;

  UPROPERTY()
  TObjectPtr<UVoxelDebug> VoxelDebug;

  UPROPERTY()
  TObjectPtr<UVoxelMesh> VoxelMesh;
};
