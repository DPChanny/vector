#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "VoxelDebugWidget.generated.h"

UCLASS()
class VOXEL_API UVoxelDebugWidget : public UUserWidget {
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintImplementableEvent, Category = "Voxel Debug")
  void UpdateInfo(const FIntVector &VoxelCoord, int32 VoxelID,
                  float CurrentDurability, float MaxDurability,
                  float CurrentDensity, float BaseDensity);
};
