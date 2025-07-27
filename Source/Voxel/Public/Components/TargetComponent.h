#pragma once

#include "CoreMinimal.h"
#include "EntityComponent.h"
#include "TargetComponent.generated.h"

UCLASS()
class VOXEL_API UTargetComponent : public UEntityComponent {
  GENERATED_BODY()

public:
  virtual void Tick(float DeltaTime) override;

  UPROPERTY(EditDefaultsOnly)
  TSubclassOf<AActor> TargetClass;

  UPROPERTY(EditDefaultsOnly)
  uint8 TargetCount = 1;

  UPROPERTY(VisibleAnywhere)
  TArray<TObjectPtr<AActor>> Targets;

protected:
  void UpdateTargets();
  virtual float GetTargetPriority(TObjectPtr<AActor> TargetCandidate);
  virtual bool IsValidTarget(AActor *Actor) const;
};