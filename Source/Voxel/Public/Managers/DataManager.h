#pragma once

#include "CoreMinimal.h"
#include "DataAssets/VoxelVoidDataAsset.h"
#include "Voxel/Public/VoxelChunk.h"

#include "DataManager.generated.h"

class UVoxelBlockDataAsset;
class UMeshManager;
class UDebugManager;
class UVoxelBorderDataAsset;
class UVoxelVoidDataAsset;
class UVoxelBaseDataAsset;
class AVoxelChunkActor;
class UEntityManager;

UCLASS()

class VOXEL_API UDataManager : public UActorComponent {
  GENERATED_BODY()

 public:
  static float GetSurfaceLevel() { return 0.f; }

  FVoxelChunk* GetVoxelChunk(const FIntVector& ChunkCoord);

  inline const FVoxelBaseData* GetVoxelData(const FIntVector& GlobalCoord);

  void ModifyVoxelData(const FIntVector& GlobalCoord,
                       const TFunction<void(FVoxelBaseData*)>& Modifier,
                       bool bAutoDebug = true);

  void SetVoxelData(const FIntVector& GlobalCoord, FVoxelBaseData* NewVoxelData,
                    bool bAutoDebug = true);

  int32 GetChunkSize() const { return ChunkSize; }

  int32 GetVoxelSize() const { return VoxelSize; }

  inline FVector GlobalToWorldCoord(const FIntVector& GlobalCoord) const;
  inline FIntVector GlobalToChunkCoord(const FIntVector& GlobalCoord) const;
  inline FIntVector GlobalToLocalCoord(const FIntVector& GlobalCoord) const;

  inline FIntVector WorldToGlobalCoord(const FVector& WorldCoord) const;
  inline FIntVector ChunkToGlobalCoord(const FIntVector& ChunkCoord) const;
  inline FIntVector LocalToGlobalCoord(const FIntVector& LocalCoord,
                                       const FIntVector& ChunkCoord) const;

  inline int32 LocalCoordToIndex(const FIntVector& LocalCoord) const;
  inline int32 GlobalCoordToIndex(const FIntVector& GlobalCoord) const;

 private:
  virtual void InitializeComponent() override;

  FVoxelChunk* LoadVoxelChunk(const FIntVector& ChunkCoord);
  void UnloadVoxelChunk(const FIntVector& ChunkCoord);

  UPROPERTY(EditDefaultsOnly)
  int32 ChunkSize = 16;

  UPROPERTY(EditDefaultsOnly)
  int32 VoxelSize = 50;

  UPROPERTY(EditDefaultsOnly)
  TObjectPtr<UVoxelBlockDataAsset> VoxelDefaultBlockDataAsset;

  UPROPERTY(EditDefaultsOnly)
  TSubclassOf<AVoxelChunkActor> VoxelChunkActorClass;

  TMap<FIntVector, FVoxelChunk> VoxelChunks;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UDebugManager> DebugManager;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UMeshManager> MeshManager;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UEntityManager> EntityManager;
};
