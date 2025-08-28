#pragma once

#include "CoreMinimal.h"
#include "DataAssets/VoxelBlockDataAsset.h"
#include "VoxelEntityDataAsset.generated.h"

UCLASS()

class UVoxelEntityDataAsset : public UVoxelBlockDataAsset {
  GENERATED_BODY()

 public:
  UPROPERTY(EditAnywhere, Category = "Voxel | Entity")
  TSubclassOf<class AEntityChunkActor> EntityChunkActorClass;

  virtual FVoxelBaseData* ConstructVoxelData(
      const FVoxelBaseParams& Params) const override;
};

USTRUCT()

struct FVoxelEntityData : public FVoxelBlockData {
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere, Category = "Voxel | Entity")
  int32 TeamID = -1;

  FVoxelEntityData() = default;

  static bool IsEntity(const FVoxelBaseData* VoxelBaseData) {
    return dynamic_cast<const FVoxelEntityData*>(VoxelBaseData) != nullptr;
  }

  explicit FVoxelEntityData(
      const TObjectPtr<const UVoxelEntityDataAsset>& InDataAsset,
      const float InDurability, const int32 InTeamID)
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

  virtual FVoxelBaseData* Clone() const override {
    return new FVoxelEntityData(*this);
  };
};

struct FVoxelEntityParams final : FVoxelBlockParams {
  int32 TeamID;

  explicit FVoxelEntityParams(const float InDurability, const int32 InTeamID)
      : FVoxelBlockParams(InDurability), TeamID(InTeamID) {}

  virtual FVoxelBaseParams* Clone() const override {
    return new FVoxelEntityParams(*this);
  }
};