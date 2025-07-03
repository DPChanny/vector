#pragma once

#include "CoreMinimal.h"
#include "VoxelSubstanceDataAsset.h"
#include "VoxelBlockDataAsset.generated.h"


class UMaterialInterface;
class UNiagaraSystem;
class USoundCue;

UCLASS(BlueprintType)
class VECTOR_API UVoxelBlockDataAsset : public UVoxelSubstanceDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Voxel | Destructible", meta = (ClampMin = "0.0"))
	float MaxDurability = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Voxel | Destructible")
	TSoftObjectPtr<UNiagaraSystem> BreakEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Voxel | Destructible")
	TSoftObjectPtr<USoundCue> BreakSound;
};