#pragma once

#include "CoreMinimal.h"
#include "EntityManager.generated.h"

class AEntityChunkActor;
struct FVoxelEntityData;

UCLASS()

class VOXEL_API UEntityManager : public UActorComponent {
  GENERATED_BODY()

  const FIntVector NeighborOffsets[26] = {
      FIntVector(1, 0, 0),   FIntVector(-1, 0, 0),  FIntVector(0, 1, 0),
      FIntVector(0, -1, 0),  FIntVector(0, 0, 1),   FIntVector(0, 0, -1),

      FIntVector(1, 1, 0),   FIntVector(1, -1, 0),  FIntVector(-1, 1, 0),
      FIntVector(-1, -1, 0), FIntVector(1, 0, 1),   FIntVector(1, 0, -1),
      FIntVector(-1, 0, 1),  FIntVector(-1, 0, -1), FIntVector(0, 1, 1),
      FIntVector(0, 1, -1),  FIntVector(0, -1, 1),  FIntVector(0, -1, -1),

      FIntVector(1, 1, 1),   FIntVector(1, 1, -1),  FIntVector(1, -1, 1),
      FIntVector(1, -1, -1), FIntVector(-1, 1, 1),  FIntVector(-1, 1, -1),
      FIntVector(-1, -1, 1), FIntVector(-1, -1, -1)};

 public:
  void SetDirtyEntity(const FIntVector& GlobalCoord);

  void OnEntityModified(const FIntVector& GlobalCoord,
                        const FVoxelEntityData& EntityData);

  void FlushDirtyEntities();

 private:
  UEntityManager();

  virtual void InitializeComponent() override;

  void MergeEntityChunk(const FIntVector& GlobalCoord,
                        const FVoxelEntityData& EntityData);

  void SplitEntityChunk(const FIntVector& GlobalCoord,
                        TSet<FIntVector>& ProcessedVoxels);

  TObjectPtr<AEntityChunkActor> ConstructEntityChunk(
      const FVoxelEntityData& EntityData);

  UPROPERTY()
  TObjectPtr<class UDataManager> DataManager;

  TMap<FIntVector, TObjectPtr<AEntityChunkActor>> EntityToChunk;

  TSet<TObjectPtr<AEntityChunkActor>> EntityChunkActors;

  TSet<FIntVector> DirtyEntityVoxels;
};
