#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelDebugActor.generated.h"

class UVoxelData;
class UBoxComponent;
class UWidgetComponent;
class UVoxelDebugWidget;

UCLASS()
class VECTOR_API AVoxelDebugActor : public AActor {
  GENERATED_BODY()

  AVoxelDebugActor();

  virtual void BeginPlay() override;

  FIntVector VoxelCoord;
  TWeakObjectPtr<UVoxelData> VoxelData;

protected:
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
  TObjectPtr<UBoxComponent> Box;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
  TObjectPtr<UWidgetComponent> Widget;

  UPROPERTY()
  TObjectPtr<UVoxelDebugWidget> DisplayWidget;

public:
  void Initialize(const FIntVector &InVoxelCoord, UVoxelData *InVoxelData);
  void UpdateWidget();
};
