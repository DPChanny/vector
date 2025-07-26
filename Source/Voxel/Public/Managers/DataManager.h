#pragma once

#include "CoreMinimal.h"
#include "DataAssets/VoxelVoidDataAsset.h"
#include "Voxel/Public/VoxelChunk.h"

// clang-format off
#include "DataManager.generated.h"
// clang-format on

class UVoxelBlockDataAsset;
class UMeshManager;
class UDebugManager;
class UVoxelBorderDataAsset;
class UVoxelVoidDataAsset;
class UVoxelBaseDataAsset;
class AVoxelChunkActor;
class UEntityManager;

UCLASS()
class VOXEL_API UDataManager : public UObject {
  GENERATED_BODY()

public:
  static float GetSurfaceLevel() { return 0.f; }

  void Initialize(const FIntVector &InWorldSizeInChunks, int32 InChunkSize,
                  int32 InVoxelSize,
                  const TSubclassOf<AVoxelChunkActor> &InVoxelChunkActor);

  void LoadVoxelChunk(const FIntVector &ChunkCoord);
  void UnloadVoxelChunk(const FIntVector &ChunkCoord);
  inline bool IsVoxelChunkLoaded(const FIntVector &ChunkCoord) const;
  FVoxelChunk *GetVoxelChunk(const FIntVector &ChunkCoord);

  inline const FVoxelBaseData *
  GetVoxelData(const FIntVector &GlobalCoord) const;

  void ModifyVoxelData(const FIntVector &GlobalCoord,
                       const TFunction<void(FVoxelBaseData *)> &Modifier,
                       bool bAutoDebug = true);

  void SetVoxelData(const FIntVector &GlobalCoord, FVoxelBaseData *NewVoxelData,
                    bool bAutoDebug = true);

  int32 GetChunkSize() const { return ChunkSize; }
  int32 GetVoxelSize() const { return VoxelSize; }

  inline FIntVector GlobalToChunkCoord(const FIntVector &GlobalCoord) const;
  inline FIntVector ChunkToGlobalCoord(const FIntVector &ChunkCoord) const;
  inline FIntVector GlobalToLocalCoord(const FIntVector &GlobalCoord) const;
  inline int32 GlobalCoordToIndex(const FIntVector &GlobalCoord) const;
  inline FIntVector LocalToGlobalCoord(const FIntVector &LocalCoord,
                                       const FIntVector &ChunkCoord) const;
  inline FIntVector WorldToGlobalCoord(const FVector &WorldCoord) const;
  inline FVector GlobalToWorldCoord(const FIntVector &GlobalCoord) const;

  inline int32 LocalCoordToIndex(const FIntVector &LocalCoord) const;

private:
  TMap<FIntVector, FVoxelChunk> VoxelChunks;
  TMap<int32, TObjectPtr<UVoxelBaseDataAsset>> VoxelDataAssets;

  UPROPERTY()
  TSubclassOf<AVoxelChunkActor> VoxelChunk;
  int32 VoxelSize;

  FIntVector WorldSizeInChunks;
  int32 ChunkSize;
  int32 ChunkVolume;

  UPROPERTY()
  TObjectPtr<UDebugManager> DebugManager;

  UPROPERTY()
  TObjectPtr<UMeshManager> MeshManager;

  UPROPERTY()
  TObjectPtr<UEntityManager> EntityManager;
};
