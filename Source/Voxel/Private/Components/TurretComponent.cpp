#include "Components/TurretComponent.h"

#include "Actors/EntityChunkActor.h"
#include "Damageable.h"

#include "Components/HealthComponent.h"
#include "Components/SurfaceComponent.h"
#include "Managers/DataManager.h"

void UTurretComponent::InitializeComponent() {
  Super::InitializeComponent();

  Timer = TimeInterval;

  HealthComponent = GetOwner()->GetComponentByClass<UHealthComponent>();
}

bool UTurretComponent::IsValidTarget(const TObjectPtr<AActor> Actor) const {
  if (!HealthComponent) {
    return false;
  }

  return FVector::Dist(Actor->GetActorLocation(),
                       GetOwner()->GetActorLocation()) <
         Amplifier(BaseRange, RangeBalancer,
                   EntityChunkActor->GetEntityCoords().Num(),
                   HealthComponent->CurrentHealth / HealthComponent->MaxHealth);
}

void UTurretComponent::TickComponent(
    const float DeltaTime, const ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  if (!HealthComponent) {
    return;
  }

  if (Timer > 0) {
    Timer -= DeltaTime;
  } else if (!Targets.IsEmpty()) {
    Timer = TimeInterval;
    const float Damage = Amplifier(
        BaseDamage, DamageBalancer, EntityChunkActor->GetEntityCoords().Num(),
        HealthComponent->CurrentHealth / HealthComponent->MaxHealth);
    for (const auto Target : Targets) {
      if (Target->GetClass()->ImplementsInterface(UDamageable::StaticClass())) {
        IDamageable::Execute_OnDamage(Target, Target->GetActorLocation(),
                                      Damage, 0);
      }
      DrawDebugLine(GetWorld(),
                    EntityChunkActor->GetDataManager()->GlobalToWorldCoord(
                        SurfaceComponent->GetClosestSurfaceVoxel(
                            Target->GetActorLocation())),
                    Target->GetActorLocation(), FColor::Red, false,
                    TimeInterval / 2, 0, .25f);
    }
  }
}

float UTurretComponent::Amplifier(const float Base, const float Balancer,
                                  const float Amount, const float Ratio) {
  return Base + FMath::Log2(Amount) * Balancer * Ratio;
}
