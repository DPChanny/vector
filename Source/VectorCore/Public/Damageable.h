#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "Damageable.generated.h"

UINTERFACE(MinimalAPI)

class UDamageable : public UInterface {
  GENERATED_BODY()
};

class VECTORCORE_API IDamageable {
  GENERATED_BODY()

 public:
  UFUNCTION(BlueprintNativeEvent)
  void OnDamage(const FVector HitPoint, const float DamageAmount,
                const float DamageRange);
};
