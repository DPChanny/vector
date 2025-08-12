#include "Components/TargetComponent.h"

#include "Actors/EntityChunkActor.h"
#include "Components/HealthComponent.h"
#include "Components/SurfaceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Managers/DataManager.h"

void UTargetComponent::InitializeComponent() {
  Super::InitializeComponent();

  SurfaceComponent = GetOwner()->GetComponentByClass<USurfaceComponent>();
}

void UTargetComponent::TickComponent(
    const float DeltaTime, const ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
  if (!TargetClass) {
    return;
  }

  UpdateTargets();
}

UTargetComponent::UTargetComponent() {
  PrimaryComponentTick.bCanEverTick = true;
}

void UTargetComponent::UpdateTargets() {
  Targets.RemoveAll([this](const TObjectPtr<AActor>& Target) {
    return !IsValidTarget(Target.Get());
  });

  TArray<AActor*> Actors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), TargetClass, Actors);

  TArray<TPair<float, TObjectPtr<AActor>>> Priorities;
  for (AActor* Actor : Actors) {
    if (IsValidTarget(Actor) && !Targets.Contains(Actor)) {
      Priorities.Add(
          TPair<float, TObjectPtr<AActor>>(GetTargetPriority(Actor), Actor));
    }
  }

  Priorities.Sort(
      [](const TPair<float, TObjectPtr<AActor>>& A,
         const TPair<float, TObjectPtr<AActor>>& B) { return A.Key < B.Key; });

  const int32 TargetsToAdd = MaxTargetCount - Targets.Num();
  for (int32 i = 0; i < FMath::Min(TargetsToAdd, Priorities.Num()); ++i) {
    Targets.Add(Priorities[i].Value);
  }
}

bool UTargetComponent::IsValidTarget(const TObjectPtr<AActor> Actor) const {
  return true;
}

float UTargetComponent::GetTargetPriority(const TObjectPtr<AActor> Target) {
  if (!Target || !SurfaceComponent) {
    return MAX_FLT;
  }

  return FVector::Dist(Target->GetActorLocation(),
                       EntityChunkActor->GetDataManager()->GlobalToWorldCoord(
                           SurfaceComponent->GetClosestSurfaceVoxel(
                               Target->GetActorLocation())));
}