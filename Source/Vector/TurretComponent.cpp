#include "TurretComponent.h"
#include "VectorPlayerCharacter.h"

#include "Components/HealthComponent.h"
#include "EntityChunk.h"

UTurretComponent::UTurretComponent() {
  TargetClass = AVectorPlayerCharacter::StaticClass();
}

bool UTurretComponent::IsValidTarget(const TObjectPtr<AActor> Actor) const {
  return FVector::Dist(Actor->GetActorLocation(), OwnerChunk->CenterOfMass) <
         Amplifier(BaseRange, RangeBalancer,
                   OwnerChunk->GetManagedVoxels().Num(),
                   HealthComponent->CurrentHealth / HealthComponent->MaxHealth);
}

void UTurretComponent::Tick(const float DeltaTime) {
  if (!CheckHealthComponent()) {
    return;
  }

  Super::Tick(DeltaTime);

  if (Timer > 0) {
    Timer -= DeltaTime;
  } else {
    Timer = TimeInterval;
    const float Damage = Amplifier(
        BaseDamage, DamageBalancer, OwnerChunk->GetManagedVoxels().Num(),
        HealthComponent->CurrentHealth / HealthComponent->MaxHealth);
    for (const auto Target : Targets) {
      if (Target->GetClass()->ImplementsInterface(UDamageable::StaticClass())) {
        IDamageable::Execute_OnDamage(Target, Target->GetActorLocation(), Damage,
                                      0);
      }
      DrawDebugLine(GetWorld(), OwnerChunk->CenterOfMass,
                    Target->GetActorLocation(), FColor::Red, false, 5.f, 0,
                    .5f);
    }
  }
}

float UTurretComponent::Amplifier(const float Base, const float Balancer,
                                  const float Amount, const float Ratio) {
  return Base + FMath::Log2(Amount) * Balancer * Ratio;
}

bool UTurretComponent::CheckHealthComponent() {
  if (HealthComponent) {
    return true;
  }
  if (!OwnerChunk) {
    return false;
  }
  HealthComponent = OwnerChunk->GetComponent<UHealthComponent>();
  return HealthComponent != nullptr;
}