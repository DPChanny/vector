#pragma once

#include "CoreMinimal.h"
#include "EntityManager.generated.h"

class UEntityChunk;
struct FVoxelEntityData;

UCLASS()

class VOXEL_API UEntityManager : public UActorComponent {
  GENERATED_BODY()

  const FIntVector NeighborOffsets[6] = {
      FIntVector(1, 0, 0),  FIntVector(-1, 0, 0), FIntVector(0, 1, 0),
      FIntVector(0, -1, 0), FIntVector(0, 0, 1),  FIntVector(0, 0, -1)};

 public:
  virtual void TickComponent(
      float DeltaTime, ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction) override;

  void OnEntityDataCreated(const FIntVector& GlobalCoord,
                           const FVoxelEntityData& EntityData);
  void OnEntityDataDestroyed(const FIntVector& GlobalCoord,
                             const FVoxelEntityData& EntityData);

  void OnEntityDataModified(const FIntVector& GlobalCoord,
                            const FVoxelEntityData& EntityData);

 private:
  virtual void BeginPlay() override;

  void UpdateEntityChunk(const TObjectPtr<UEntityChunk>& OriginalChunk);
  bool GetChunkableEntityCoords(const FIntVector& StartCoord,
                                TSet<FIntVector>& VisitedCoords,
                                TSet<FIntVector>& ChunkableEntityCoords) const;
  TObjectPtr<UEntityChunk> CreateEntityChunk(
      const FVoxelEntityData& EntityData);

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<class UDataManager> DataManager;

  TMap<FIntVector, TObjectPtr<UEntityChunk>> EntityToChunk;

  TSet<TObjectPtr<UEntityChunk>> EntityChunks;
};
