#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelDebugActor.generated.h"

class AVoxelWorld;
class UBoxComponent;
class UWidgetComponent;
class UVoxelDebugWidget;

UCLASS()
class VECTOR_API AVoxelDebugActor : public AActor
{
	GENERATED_BODY()

private:
	AVoxelDebugActor();

	virtual void BeginPlay() override;

	FIntVector VoxelCoord;
	TWeakObjectPtr<AVoxelWorld> World;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> Box;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> Widget;

	UPROPERTY()
	TObjectPtr<UVoxelDebugWidget> DisplayWidget;

public:
	void Initialize(const FIntVector& InVoxelCoord);
	void UpdateWidget();

};
