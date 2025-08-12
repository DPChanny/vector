#pragma once

#include "CoreMinimal.h"
#include "EntityChunkActor.generated.h"

class UDataManager;
class UEntityComponent;
struct FVoxelEntityData;

UCLASS()

class VOXEL_API AEntityChunkActor : public AActor {
  GENERATED_BODY()

 public:
  virtual void AddEntity(const FIntVector& GlobalCoord,
                         const FVoxelEntityData& NewEntityData);
  virtual void RemoveEntity(const FIntVector& GlobalCoord);

  bool IsEmpty() const;

  virtual void OnEntityModified(const FIntVector& GlobalCoord,
                                const FVoxelEntityData& NewEntityData);

  const TSet<FIntVector>& GetEntityCoords() const { return EntityCoords; }

  bool IsChunkableWith(const FVoxelEntityData& Other) const;

  FVoxelEntityData* GetCache(const FIntVector& GlobalCoord);

  TObjectPtr<UDataManager>& GetDataManager() { return DataManager; }

  void SetCacheEnabled(bool InbCacheEnabled);

 protected:
  bool bCacheEnabled = false;

  virtual void BeginPlay() override;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UDataManager> DataManager;

  TSet<FIntVector> EntityCoords;

  TArray<TObjectPtr<UEntityComponent>> Components;

  void UpdateLocation();
  TMap<FIntVector, FVoxelEntityData*> Cache;
};
