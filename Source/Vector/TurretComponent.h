#pragma once

#include "Components/TargetComponent.h"
#include "CoreMinimal.h"
#include "TurretComponent.generated.h"

class UHealthComponent;
UCLASS()

class VECTOR_API UTurretComponent : public UTargetComponent {
  GENERATED_BODY()

  UTurretComponent();

 public:
  virtual bool IsValidTarget(TObjectPtr<AActor> Actor) const override;
  virtual void Tick(float DeltaTime) override;

 private:
  static float Amplifier(float Base, float Balancer, float Amount, float Ratio);

  bool CheckHealthComponent();

 protected:
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
  float TimeInterval = 5.f;

  UPROPERTY(EditDefaultsOnly)
  float BaseDamage = 50.f;
};
