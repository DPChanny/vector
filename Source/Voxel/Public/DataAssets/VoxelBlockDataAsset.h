#pragma once

#include "CoreMinimal.h"
#include "VoxelSubstanceDataAsset.h"

// clang-format off
#include "VoxelBlockDataAsset.generated.h"
// clang-format on

class UMaterialInterface;
class UNiagaraSystem;
class USoundCue;

UCLASS()
class VOXEL_API UVoxelBlockDataAsset : public UVoxelSubstanceDataAsset {
  GENERATED_BODY()

public:
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Voxel | Destructible", meta = (ClampMin = "0.0"))
  float MaxDurability = 100.0f;

  virtual FVoxelBaseData *
  ConstructVoxelData(const FVoxelBaseParams &Params) const override;
};

USTRUCT()
struct FVoxelBlockData : public FVoxelSubstanceData {
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere)
  float Durability = 0.f;

  FVoxelBlockData() = default;

  static bool IsBlock(const FVoxelBaseData *VoxelBaseData) {
    return dynamic_cast<const FVoxelBlockData *>(VoxelBaseData) != nullptr;
  }

  explicit FVoxelBlockData(
      const TObjectPtr<const UVoxelBlockDataAsset> &InDataAsset,
      const float InDurability)
      : FVoxelSubstanceData(InDataAsset), Durability(InDurability) {}

  TObjectPtr<const UVoxelBlockDataAsset> GetBlockDataAsset() const {
    return Cast<const UVoxelBlockDataAsset>(DataAsset);
  }

  virtual float GetDensity() const override {
    return DataAsset ? DataAsset->BaseDensity * Durability /
                           GetBlockDataAsset()->MaxDurability
                     : -1.f;
  }
};

struct FVoxelBlockParams : FVoxelSubstanceParams {
  float Durability;

  explicit FVoxelBlockParams(const float InDurability)
      : Durability(InDurability) {}

  virtual FVoxelBaseParams *Clone() const override {
    return new FVoxelBlockParams(*this);
  }
};
