#include "Components/TargetComponent.h"

#include "EntityChunk.h"
#include "Kismet/GameplayStatics.h"

void UTargetComponent::Tick(const float DeltaTime) {
  Super::Tick(DeltaTime);

  if (!TargetClass || !OwnerChunk) {
    return;
  }

  UpdateTargets();
}

void UTargetComponent::UpdateTargets() {
  Targets.RemoveAll([this](const TObjectPtr<AActor> &Target) {
    return !IsValidTarget(Target.Get());
  });

  if (Targets.Num() >= TargetCount) {
    return;
  }

  TArray<AActor *> Actors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), TargetClass, Actors);

  TArray<TPair<float, TObjectPtr<AActor>>> Priorities;
  for (AActor *Actor : Actors) {
    if (IsValidTarget(Actor) && !Targets.Contains(Actor)) {
      float Priority = GetTargetPriority(Actor);
      Priorities.Add(TPair<float, TObjectPtr<AActor>>(Priority, Actor));
    }
  }

  Priorities.Sort(
      [](const TPair<float, TObjectPtr<AActor>> &A,
         const TPair<float, TObjectPtr<AActor>> &B) { return A.Key < B.Key; });

  const int32 TargetsToAdd = TargetCount - Targets.Num();
  for (int32 i = 0; i < FMath::Min(TargetsToAdd, Priorities.Num()); ++i) {
    Targets.Add(Priorities[i].Value);
  }
}

bool UTargetComponent::IsValidTarget(AActor *Actor) const {
  return Actor && IsValid(Actor) && Actor->IsA(TargetClass);
}

float UTargetComponent::GetTargetPriority(
    const TObjectPtr<AActor> TargetCandidate) {
  if (!TargetCandidate || !OwnerChunk) {
    return MAX_FLT;
  }

  return FVector::Dist(TargetCandidate->GetActorLocation(),
                       OwnerChunk->CenterOfMass);
}