#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelDebugActor.generated.h"

class UVoxelData;
class UBoxComponent;
class UWidgetComponent;
class UVoxelDebugWidget;

UCLASS()
class VOXEL_API AVoxelDebugActor : public AActor {
  GENERATED_BODY()

  AVoxelDebugActor();

  virtual void BeginPlay() override;

  FIntVector VoxelCoord;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<const UVoxelData> VoxelData;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UBoxComponent> Box;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UWidgetComponent> Widget;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UVoxelDebugWidget> DisplayWidget;

public:
  void Initialize(const FIntVector &InVoxelCoord);
  void UpdateWidget() const;
};
