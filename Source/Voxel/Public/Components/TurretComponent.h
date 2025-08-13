#pragma once

#include "Components/TargetComponent.h"
#include "CoreMinimal.h"
#include "TurretComponent.generated.h"

class UHealthComponent;

UCLASS(meta = (BlueprintSpawnableComponent))

class VOXEL_API UTurretComponent : public UTargetComponent {
  GENERATED_BODY()

 public:
  virtual bool IsValidTarget(TObjectPtr<AActor> Actor) const override;
  virtual void TickComponent(
      float DeltaTime, ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction) override;

 protected:
  static float Amplifier(float Base, float Balancer, float Amount, float Ratio);

  virtual void InitializeComponent() override;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<const UHealthComponent> HealthComponent;

  UPROPERTY(EditDefaultsOnly)
  float RangeBalancer = 5.f;

  UPROPERTY(EditDefaultsOnly)
  float DamageBalancer = 5.f;

  UPROPERTY(EditDefaultsOnly)
  float BaseRange = 200.f;

  float Timer = 0.f;

  UPROPERTY(EditDefaultsOnly)
  float TimeInterval = 2.5f;

  UPROPERTY(EditDefaultsOnly)
  float BaseDamage = 50.f;
};
