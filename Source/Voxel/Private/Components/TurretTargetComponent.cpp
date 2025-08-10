#include "Components/TurretTargetComponent.h"

#include "Components/HealthComponent.h"
#include "EntityChunk.h"

bool UTurretTargetComponent::IsValidTarget(
    const TObjectPtr<AActor> Actor) const {

  return FVector::Dist(Actor->GetActorLocation(), OwnerChunk->CenterOfMass) <
         200.f;
}

void UTurretTargetComponent::Tick(const float DeltaTime) {
  Super::Tick(DeltaTime);
  if (Timer > 0) {
    Timer -= DeltaTime;
  } else {
    Timer = TimeInterval;
    float Damage = BaseDamage;
    if (const TObjectPtr<const UHealthComponent> HealthComponent =
            OwnerChunk->GetComponent<UHealthComponent>()) {
      Damage *= HealthComponent->CurrentHealth / HealthComponent->TotalHealth;
    }
    for (const auto Target : Targets) {
      UE_LOG(LogTemp, Log, TEXT("Attacking %s with damage amount of %f"),
             *Target->GetName(), Damage);
    }
  }
}