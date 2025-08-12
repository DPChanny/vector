#include "TurretComponent.h"
#include "Actors/EntityChunkActor.h"
#include "VectorPlayerCharacter.h"

#include "Components/HealthComponent.h"

UTurretComponent::UTurretComponent() : Entities(nullptr) {
  TargetClass = AVectorPlayerCharacter::StaticClass();
}

bool UTurretComponent::IsValidTarget(const TObjectPtr<AActor> Actor) const {
  return FVector::Dist(Actor->GetActorLocation(),
                       GetOwner()->GetActorLocation()) <
         Amplifier(BaseRange, RangeBalancer, Entities->Num(),
                   HealthComponent->CurrentHealth / HealthComponent->MaxHealth);
}

void UTurretComponent::TickComponent(
    const float DeltaTime, const ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction) {
  if (!CheckHealthComponent()) {
    return;
  }

  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  if (Timer > 0) {
    Timer -= DeltaTime;
  } else if (!Targets.IsEmpty()) {
    Timer = TimeInterval;
    const float Damage =
        Amplifier(BaseDamage, DamageBalancer, Entities->Num(),
                  HealthComponent->CurrentHealth / HealthComponent->MaxHealth);
    for (const auto Target : Targets) {
      if (Target->GetClass()->ImplementsInterface(UDamageable::StaticClass())) {
        IDamageable::Execute_OnDamage(Target, Target->GetActorLocation(),
                                      Damage, 0);
      }
      DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(),
                    Target->GetActorLocation(), FColor::Red, false,
                    TimeInterval / 2, 0, .25f);
    }
  }
}

void UTurretComponent::InitializeComponent() {
  Super::InitializeComponent();

  Entities = &EntityChunkActor->GetEntities();
  Timer = TimeInterval;
}

float UTurretComponent::Amplifier(const float Base, const float Balancer,
                                  const float Amount, const float Ratio) {
  return Base + FMath::Log2(Amount) * Balancer * Ratio;
}

bool UTurretComponent::CheckHealthComponent() {
  if (HealthComponent) {
    return true;
  }

  HealthComponent = GetOwner()->GetComponentByClass<UHealthComponent>();
  return HealthComponent != nullptr;
}