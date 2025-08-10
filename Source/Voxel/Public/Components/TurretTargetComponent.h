#pragma once

#include "Components/TargetComponent.h"
#include "CoreMinimal.h"
#include "TurretTargetComponent.generated.h"

UCLASS()

class VOXEL_API UTurretTargetComponent : public UTargetComponent {
  GENERATED_BODY()

 public:
  virtual bool IsValidTarget(TObjectPtr<AActor> Actor) const override;
  virtual void Tick(float DeltaTime) override;

 private:
  float Timer = 0.f;

  UPROPERTY()
  float TimeInterval = 5.f;

  UPROPERTY()
  float BaseDamage = 100.f;
};
