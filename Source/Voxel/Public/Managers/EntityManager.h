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
  void OnEntityDataCreated(const FIntVector& GlobalCoord,
                           const FVoxelEntityData& EntityData);
  void OnEntityDataDestroyed(const FIntVector& GlobalCoord,
                             const FVoxelEntityData& EntityData);

  void OnEntityDataModified(const FIntVector& GlobalCoord,
                            const FVoxelEntityData& EntityData);

 private:
  UEntityManager();

  virtual void InitializeComponent() override;

  void UpdateEntityChunk(const TObjectPtr<AEntityChunkActor>& OriginalChunk);
  bool GetChunkableEntityCoords(const FIntVector& StartCoord,
                                TSet<FIntVector>& VisitedCoords,
                                TSet<FIntVector>& ChunkableEntityCoords) const;
  TObjectPtr<AEntityChunkActor> CreateEntityChunk(
      const FVoxelEntityData& EntityData);

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<class UDataManager> DataManager;

  TMap<FIntVector, TObjectPtr<AEntityChunkActor>> EntityToChunk;

  TSet<TObjectPtr<AEntityChunkActor>> EntityChunks;
};
