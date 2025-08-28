#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

UCLASS()

class LOBBY_API ALobbyGameMode : public AGameModeBase {
  GENERATED_BODY()

 public:
  UFUNCTION(BlueprintCallable)
  void StartGame() const;
};