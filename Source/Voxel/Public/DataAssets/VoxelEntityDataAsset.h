#pragma once

#include "CoreMinimal.h"
#include "DataAssets/VoxelBlockDataAsset.h"
#include "VoxelEntityDataAsset.generated.h"

class UEntityChunk;

UCLASS()
class UVoxelEntityDataAsset : public UVoxelBlockDataAsset {
  GENERATED_BODY()

public:
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity")
  TSubclassOf<UEntityChunk> EntityChunkClass;
};

USTRUCT()
struct FVoxelEntityData : public FVoxelBlockData {
  GENERATED_BODY()

  FVoxelEntityData() = default;

  static bool IsEntity(const FVoxelBaseData *VoxelBaseData) {
    return dynamic_cast<const FVoxelEntityData *>(VoxelBaseData) != nullptr;
  }

  explicit FVoxelEntityData(const TObjectPtr<UVoxelEntityDataAsset> &InPtr,
                            const float InDurability)
      : FVoxelBlockData(InPtr, InDurability) {}

  TObjectPtr<UVoxelEntityDataAsset> GetEntityDataAsset() const {
    return Cast<UVoxelEntityDataAsset>(DataAsset);
  }

  virtual bool IsIdentical(const FVoxelEntityData *Other) const {
    if (!Other || !DataAsset) {
      return false;
    }

    return GetEntityDataAsset()->GetClass() ==
           Other->GetEntityDataAsset()->GetClass();
  }

  virtual float GetDensity() const override {
    return DataAsset ? DataAsset->BaseDensity * Durability /
                           GetBlockDataAsset()->MaxDurability
                     : -1.f;
  }
};