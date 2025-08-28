#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "LobbyPlayerState.generated.h"

UCLASS()

class LOBBY_API ALobbyPlayerState : public APlayerState {
  GENERATED_BODY()

 public:
  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  UPROPERTY(Replicated, VisibleAnywhere)
  int32 TeamID = -1;
};
