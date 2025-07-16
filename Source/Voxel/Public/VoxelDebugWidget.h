#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "VoxelDebugWidget.generated.h"

UCLASS()
class VOXEL_API UVoxelDebugWidget : public UUserWidget {
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintImplementableEvent, Category = "Voxel Debug")
  void UpdateText(const FString &Text);
};
