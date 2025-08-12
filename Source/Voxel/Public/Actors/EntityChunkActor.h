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
  virtual void AddEntity(const FIntVector& VoxelCoord,
                         const FVoxelEntityData& EntityData);
  virtual void RemoveEntity(const FIntVector& VoxelCoord,
                            const FVoxelEntityData& EntityData);

  bool IsEmpty() const;

  virtual void OnEntityDataModified(const FIntVector& VoxelCoord,
                                    const FVoxelEntityData& EntityData);

  const TSet<FIntVector>& GetManagedVoxels() const { return ManagedVoxels; }

 private:
  virtual void BeginPlay() override;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UDataManager> DataManager;

  TSet<FIntVector> ManagedVoxels;

  TArray<TObjectPtr<UEntityComponent>> Components;

  void UpdateLocation();
};
