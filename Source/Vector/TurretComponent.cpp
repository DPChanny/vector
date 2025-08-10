#include "TurretComponent.h"
#include "VectorPlayerCharacter.h"

#include "Components/HealthComponent.h"
#include "EntityChunk.h"

UTurretComponent::UTurretComponent() {
  TargetClass = AVectorPlayerCharacter::StaticClass();
}

bool UTurretComponent::IsValidTarget(
    const TObjectPtr<AActor> Actor) const {
  return FVector::Dist(Actor->GetActorLocation(), OwnerChunk->CenterOfMass) <
         BaseRange;
}

void UTurretComponent::Tick(const float DeltaTime) {
  Super::Tick(DeltaTime);
  if (Timer > 0) {
    Timer -= DeltaTime;
  } else {
    Timer = TimeInterval;
    float Damage = BaseDamage;
    if (const TObjectPtr<const UHealthComponent> HealthComponent =
            OwnerChunk->GetComponent<UHealthComponent>()) {
      Damage *= 1.f;
    }
    for (const auto Target : Targets) {
      UE_LOG(LogTemp, Log, TEXT("Attacking %s with damage amount of %f"),
             *Target->GetName(), Damage);
    }
  }
}