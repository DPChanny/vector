#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VoxelBaseDataAsset.generated.h"

UCLASS(Abstract)

class VOXEL_API UVoxelBaseDataAsset : public UDataAsset {
  GENERATED_BODY()

 public:
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Voxel | Base")
  FString VoxelName = "None";

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel | Base",
            meta = (ClampMin = "-1.0", ClampMax = "1.0"))
  float BaseDensity = 1.0f;

  virtual struct FVoxelBaseData* ConstructVoxelData(
      const struct FVoxelBaseParams& Params) const;
};

USTRUCT()

struct FVoxelBaseData {
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<const UVoxelBaseDataAsset> DataAsset;

  FVoxelBaseData() = default;

  explicit FVoxelBaseData(
      const TObjectPtr<const UVoxelBaseDataAsset>& InDataAsset)
      : DataAsset(InDataAsset) {}

  virtual float GetDensity() const {
    return DataAsset ? DataAsset->BaseDensity : -1.f;
  }

  virtual FVoxelBaseData* Clone() const { return nullptr; };

  virtual ~FVoxelBaseData() = default;
};

struct FVoxelBaseParams {
  virtual ~FVoxelBaseParams() = default;

  virtual FVoxelBaseParams* Clone() const = 0;
};