#pragma once

#include "CoreMinimal.h"
#include "EntityChunk.generated.h"

class UDataManager;
class UEntityComponent;
struct FVoxelEntityData;

UCLASS()

class VOXEL_API UEntityChunk : public UObject {
  GENERATED_BODY()

 public:
  virtual void Initialize(TObjectPtr<UDataManager> InDataManager);

  virtual void Tick(float DeltaTime);

  virtual void AddEntity(const FIntVector& VoxelCoord,
                         const FVoxelEntityData& EntityData);
  virtual void RemoveEntity(const FIntVector& VoxelCoord,
                            const FVoxelEntityData& EntityData);

  bool IsEmpty() const;

  virtual void OnEntityDataModified(const FIntVector& VoxelCoord,
                                    const FVoxelEntityData& EntityData);

  const TSet<FIntVector>& GetManagedVoxels() const { return ManagedVoxels; }

  void AddComponent(const TObjectPtr<UEntityComponent>& Component);

  template <typename T>
  TObjectPtr<T> GetComponent() const {
    for (auto Component : Components) {
      if (TObjectPtr<T> Casted = Cast<T>(Component)) {
        return Casted;
      }
    }
    return nullptr;
  }

  UPROPERTY(VisibleAnywhere)
  FVector CenterOfMass;

 private:
  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UDataManager> DataManager;

  TSet<FIntVector> ManagedVoxels;

  UPROPERTY()
  TArray<TObjectPtr<UEntityComponent>> Components;

  void UpdateCenterOfMass();
};
