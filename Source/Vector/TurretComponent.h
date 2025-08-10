#pragma once

#include "Components/TargetComponent.h"
#include "CoreMinimal.h"
#include "TurretComponent.generated.h"

UCLASS()

class VECTOR_API UTurretComponent : public UTargetComponent {
  GENERATED_BODY()

  UTurretComponent();

 public:
  virtual bool IsValidTarget(TObjectPtr<AActor> Actor) const override;
  virtual void Tick(float DeltaTime) override;

 private:
  UPROPERTY(VisibleAnywhere)
  float BaseRange = 200.f;

  float Timer = 0.f;

  UPROPERTY(VisibleAnywhere)
  float TimeInterval = 5.f;

  UPROPERTY(VisibleAnywhere)
  float BaseDamage = 100.f;
};
