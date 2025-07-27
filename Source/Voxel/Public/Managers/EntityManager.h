#pragma once

#include "CoreMinimal.h"
#include "EntityManager.generated.h"

class UEntityChunk;
struct FVoxelEntityData;

UCLASS()
class VOXEL_API UEntityManager : public UObject {
  GENERATED_BODY()

  const FIntVector NeighborOffsets[6] = {
      FIntVector(1, 0, 0),  FIntVector(-1, 0, 0), FIntVector(0, 1, 0),
      FIntVector(0, -1, 0), FIntVector(0, 0, 1),  FIntVector(0, 0, -1)};

public:
  void Initialize();
  void Tick(float DeltaTime);

  void OnEntityDataCreated(const FIntVector &GlobalCoord,
                           const FVoxelEntityData &EntityData);
  void OnEntityDataDestroyed(const FIntVector &GlobalCoord,
                             const FVoxelEntityData &EntityData);

  void OnEntityDataModified(const FIntVector &GlobalCoord,
                            const FVoxelEntityData &EntityData);

private:
  void UpdateEntityChunk(const TObjectPtr<UEntityChunk> &OriginalChunk);
  bool GetChunkableEntityCoords(const FIntVector &StartCoord,
                                TSet<FIntVector> &VisitedCoords,
                                TSet<FIntVector> &ChunkableEntityCoords) const;
  TObjectPtr<UEntityChunk>
  CreateEntityChunk(const FVoxelEntityData &EntityData);

  UPROPERTY()
  TObjectPtr<class UDataManager> DataManager;

  UPROPERTY()
  TMap<FIntVector, TObjectPtr<UEntityChunk>> EntityToChunk;

  UPROPERTY()
  TSet<TObjectPtr<UEntityChunk>> EntityChunks;
};
