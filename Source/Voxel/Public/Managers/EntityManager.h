#pragma once

#include "CoreMinimal.h"
#include "EntityManager.generated.h"

class AEntityChunkActor;
struct FVoxelEntityData;

UCLASS()

class VOXEL_API UEntityManager : public UActorComponent {
  GENERATED_BODY()

  const FIntVector NeighborOffsets[6] = {
      FIntVector(1, 0, 0),  FIntVector(-1, 0, 0), FIntVector(0, 1, 0),
      FIntVector(0, -1, 0), FIntVector(0, 0, 1),  FIntVector(0, 0, -1)};

 public:
  void SetDirtyEntity(const FIntVector& GlobalCoord);

  void OnEntityModified(const FIntVector& GlobalCoord,
                        const FVoxelEntityData& EntityData);

  void FlushDirtyEntities();

 private:
  UEntityManager();

  virtual void InitializeComponent() override;

  void MergeEntityChunk(const FIntVector& GlobalCoord,
                        const FVoxelEntityData& EntityData,
                        const TSet<FIntVector>& ProcessedVoxels,
                        TSet<FIntVector>& MergedVoxels);

  void SplitEntityChunk(const FIntVector& GlobalCoord,
                        const TSet<FIntVector>& ProcessedVoxels,
                        TSet<FIntVector>& SplitVoxels);

  bool GetChunkableEntityCoords(const FIntVector& StartCoord,
                                TSet<FIntVector>& VisitedCoords,
                                TSet<FIntVector>& ChunkableEntityCoords) const;
  TObjectPtr<AEntityChunkActor> GetEntityChunk(
      const FVoxelEntityData& EntityData);

  UPROPERTY()
  TObjectPtr<class UDataManager> DataManager;

  TMap<FIntVector, TObjectPtr<AEntityChunkActor>> EntityToChunk;

  TSet<TObjectPtr<AEntityChunkActor>> EntityChunks;

  TSet<FIntVector> DirtyEntityVoxels;
};
