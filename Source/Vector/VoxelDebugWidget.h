#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VoxelDebugWidget.generated.h"

UCLASS()
class VECTOR_API UVoxelDebugWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintImplementableEvent, Category = "Voxel Debug")
    void UpdateInfo(
        const FIntVector& VoxelCoord,
        int32 VoxelID,
        float CurrentDurability,
        float MaxDurability,
        float CurrentDensity,
        float BaseDensity
    );
};
