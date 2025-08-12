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
  virtual void TickComponent(
      float DeltaTime, enum ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction) override;

 private:
  virtual void InitializeComponent() override;

  static float Amplifier(float Base, float Balancer, float Amount, float Ratio);

  bool CheckHealthComponent();

 protected:
  UPROPERTY(VisibleAnywhere)
  TObjectPtr<const UHealthComponent> HealthComponent;

  const TSet<FIntVector>* ManagedVoxels;

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
