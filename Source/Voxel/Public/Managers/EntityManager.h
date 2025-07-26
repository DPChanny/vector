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

private:
  void UpdateEntityChunk(const TObjectPtr<UEntityChunk> &OriginalChunk);
  TSet<FIntVector> GetChunkableVoxels(const FIntVector &StartCoord,
                                      TSet<FIntVector> &VisitedVoxels) const;
  TObjectPtr<UEntityChunk>
  CreateEntityChunk(const FVoxelEntityData *EntityData);

  UPROPERTY()
  TObjectPtr<class UDataManager> DataManager;

  UPROPERTY()
  TMap<FIntVector, TObjectPtr<UEntityChunk>> EntityToChunk;

  UPROPERTY()
  TSet<TObjectPtr<UEntityChunk>> EntityChunks;
};
