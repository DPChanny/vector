#pragma once

#include "CoreMinimal.h"
#include "VoxelBaseDataAsset.h"
#include "VoxelSubstanceDataAsset.generated.h"

UCLASS()
class VECTOR_API UVoxelSubstanceDataAsset : public UVoxelBaseDataAsset
{
	GENERATED_BODY()

public:
	UVoxelSubstanceDataAsset();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Voxel | Substance")
	TObjectPtr<UMaterialInterface> Material;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Voxel | Substance")
	TSoftObjectPtr<USoundCue> FootstepSound;
};
