#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelDebugActor.generated.h"

class UDataManager;
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
  TObjectPtr<const UDataManager> DataManager;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UBoxComponent> Box;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UWidgetComponent> Widget;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UVoxelDebugWidget> DisplayWidget;

public:
  void Initialize(const FIntVector &InVoxelCoord, const FColor &Color);
  void UpdateActor(const FColor &Color) const;
};
