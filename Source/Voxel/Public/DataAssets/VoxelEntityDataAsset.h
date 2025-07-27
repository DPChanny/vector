#pragma once

#include "CoreMinimal.h"
#include "DataAssets/VoxelBlockDataAsset.h"
#include "VoxelEntityDataAsset.generated.h"

UCLASS()
class UVoxelEntityDataAsset : public UVoxelBlockDataAsset {
  GENERATED_BODY()

public:
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity")
  TSubclassOf<class UEntityChunk> EntityChunkClass;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity")
  TArray<TSubclassOf<class UEntityComponent>> EntityComponentClasses;
};

USTRUCT()
struct FVoxelEntityData : public FVoxelBlockData {
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entity")
  uint8 TeamID;

  FVoxelEntityData() = default;

  static bool IsEntity(const FVoxelBaseData *VoxelBaseData) {
    return dynamic_cast<const FVoxelEntityData *>(VoxelBaseData) != nullptr;
  }

  explicit FVoxelEntityData(const TObjectPtr<UVoxelEntityDataAsset> &InPtr,
                            const float InDurability, const uint8 InTeamID = 0)
      : FVoxelBlockData(InPtr, InDurability), TeamID(InTeamID) {}

  TObjectPtr<UVoxelEntityDataAsset> GetEntityDataAsset() const {
    return Cast<UVoxelEntityDataAsset>(DataAsset);
  }

  virtual bool IsChunkableWith(const FVoxelEntityData *Other) const {
    if (!Other || !DataAsset) {
      return false;
    }

    return GetEntityDataAsset()->GetClass() ==
               Other->GetEntityDataAsset()->GetClass() &&
           TeamID == Other->TeamID;
  }

  virtual float GetDensity() const override {
    return DataAsset ? DataAsset->BaseDensity * Durability /
                           GetBlockDataAsset()->MaxDurability
                     : -1.f;
  }
};