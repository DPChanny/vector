#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "VectorPlayerState.generated.h"

UCLASS()
class VECTOR_API AVectorPlayerState : public APlayerState {
  GENERATED_BODY()

public:
  virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  UPROPERTY(Replicated, VisibleAnywhere)
  int32 TeamID = -1;
};
