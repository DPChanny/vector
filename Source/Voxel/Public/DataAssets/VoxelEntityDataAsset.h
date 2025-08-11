#pragma once

#include "CoreMinimal.h"
#include "DataAssets/VoxelBlockDataAsset.h"
#include "VoxelEntityDataAsset.generated.h"

UCLASS()

class UVoxelEntityDataAsset : public UVoxelBlockDataAsset {
  GENERATED_BODY()

 public:
  UPROPERTY(EditAnywhere, Category = "Voxel | Entity")
  TSubclassOf<class UEntityChunk> EntityChunkClass;

  UPROPERTY(EditAnywhere, Category = "Voxel | Entity")
  TArray<TSubclassOf<class UEntityComponent>> EntityComponentClasses;

  virtual FVoxelBaseData* ConstructVoxelData(
      const FVoxelBaseParams& Params) const override;
};

USTRUCT()

struct FVoxelEntityData : public FVoxelBlockData {
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere, Category = "Voxel | Entity")
  uint8 TeamID = 0;

  FVoxelEntityData() = default;

  static bool IsEntity(const FVoxelBaseData* VoxelBaseData) {
    return dynamic_cast<const FVoxelEntityData*>(VoxelBaseData) != nullptr;
  }

  explicit FVoxelEntityData(
      const TObjectPtr<const UVoxelEntityDataAsset>& InDataAsset,
      const float InDurability, const uint8 InTeamID)
      : FVoxelBlockData(InDataAsset, InDurability), TeamID(InTeamID) {}

  TObjectPtr<const UVoxelEntityDataAsset> GetEntityDataAsset() const {
    return Cast<const UVoxelEntityDataAsset>(DataAsset);
  }

  virtual bool IsChunkableWith(const FVoxelEntityData* Other) const {
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

struct FVoxelEntityParams final : FVoxelBlockParams {
  uint8 TeamID;

  explicit FVoxelEntityParams(const float InDurability, const uint8 InTeamID)
      : FVoxelBlockParams(InDurability), TeamID(InTeamID) {}

  virtual FVoxelBaseParams* Clone() const override {
    return new FVoxelEntityParams(*this);
  }
};