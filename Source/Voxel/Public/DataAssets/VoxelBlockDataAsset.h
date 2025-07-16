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
};

USTRUCT()
struct FVoxelBlockData : public FVoxelSubstanceData {
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere)
  float Durability;

  FVoxelBlockData() = default;

  static bool IsBlock(const FVoxelBaseData *VoxelBaseData) {
    return dynamic_cast<const FVoxelBlockData *>(VoxelBaseData) != nullptr;
  }

  explicit FVoxelBlockData(const TObjectPtr<UVoxelBlockDataAsset> &InPtr,
                           const float InDurability)
      : FVoxelSubstanceData(InPtr), Durability(InDurability) {}

  TObjectPtr<UVoxelBlockDataAsset> GetBlockDataAsset() const {
    return Cast<UVoxelBlockDataAsset>(DataAsset);
  }

  virtual float GetDensity() const override {
    return DataAsset ? DataAsset->BaseDensity * Durability /
                           GetBlockDataAsset()->MaxDurability
                     : -1.f;
  }
};