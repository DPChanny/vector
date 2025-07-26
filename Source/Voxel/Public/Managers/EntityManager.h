#pragma once

#include "CoreMinimal.h"
#include "EntityManager.generated.h"

class UEntityChunk;
struct FVoxelEntityData;

UCLASS()
class VOXEL_API UEntityManager : public UObject {
  GENERATED_BODY()

public:
  void Initialize();
  void Tick(float DeltaTime);

  void OnEntityCreated(const FIntVector &GlobalCoord,
                       const FVoxelEntityData *EntityData);
  void OnEntityDestroyed(const FIntVector &GlobalCoord);

  void OnEntityModified(const FIntVector &GlobalCoord);

  void UpdateVoxelChunkMapping(const FIntVector &GlobalCoord,
                               UEntityChunk *Chunk);

private:
  void RebuildChunksAround(const FIntVector &GlobalCoord);

  UEntityChunk *GetEntityChunk(const FIntVector &GlobalCoord,
                               const FVoxelEntityData *EntityData);
  UEntityChunk *CreateEntityChunk(const FVoxelEntityData *EntityData);

  UPROPERTY()
  TObjectPtr<class UDataManager> DataManager;

  UPROPERTY()
  TMap<FIntVector, TObjectPtr<UEntityChunk>> EntityToChunk;

  UPROPERTY()
  TSet<TObjectPtr<UEntityChunk>> EntityChunks;
};
