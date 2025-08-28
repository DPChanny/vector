#pragma once

#include "CoreMinimal.h"
#include "EntityComponent.h"
#include "TargetComponent.generated.h"

class USurfaceComponent;

UCLASS(meta = (BlueprintSpawnableComponent))

class VOXEL_API UTargetComponent : public UEntityComponent {
  GENERATED_BODY()

 public:
  virtual void TickComponent(
      float DeltaTime, ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction) override;

  UPROPERTY(EditDefaultsOnly)
  TSubclassOf<AActor> TargetClass;

  UPROPERTY(EditDefaultsOnly)
  int32 MaxTargetCount = 1;

 protected:
  virtual void InitializeComponent() override;

  UTargetComponent();

  TArray<TObjectPtr<AActor>> Targets;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<USurfaceComponent> SurfaceComponent;

  void UpdateTargets();
  virtual float GetTargetPriority(TObjectPtr<AActor> Target);
  virtual bool IsValidTarget(TObjectPtr<AActor> Actor) const;
};