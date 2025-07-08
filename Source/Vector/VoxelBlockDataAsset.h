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
class VECTOR_API UVoxelBlockDataAsset : public UVoxelSubstanceDataAsset {
  GENERATED_BODY()

 public:
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Voxel | Destructible", meta = (ClampMin = "0.0"))
  float MaxDurability = 100.0f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Voxel | Destructible")
  TSoftObjectPtr<UNiagaraSystem> BreakEffect;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Voxel | Destructible")
  TSoftObjectPtr<USoundCue> BreakSound;
};