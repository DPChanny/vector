#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VoxelBaseDataAsset.generated.h"

UCLASS()
class VECTOR_API UVoxelBaseDataAsset : public UDataAsset {
  GENERATED_BODY()

 public:
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Voxel | Base")
  FString VoxelName = "None";

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel | Base",
            meta = (ClampMin = "-1.0", ClampMax = "1.0"))
  float BaseDensity = 1.0f;
};
