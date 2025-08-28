#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

class AVectorPlayerState;
class ALobbyGameState;

UCLASS()

class LOBBY_API ALobbyPlayerController : public APlayerController {
  GENERATED_BODY()

  UPROPERTY()
  TObjectPtr<ALobbyGameState> LobbyGameState;

  UPROPERTY()
  TObjectPtr<AVectorPlayerState> VectorPlayerState;

  virtual void BeginPlay() override;

 public:
  UFUNCTION(BlueprintCallable, Server, Reliable)
  void ServerAddTeam(const FString& TeamName, const FString& Password) const;

  UFUNCTION(BlueprintCallable, Server, Reliable)
  void ServerJoinTeam(const int32& TeamID, const FString& Password) const;

  UFUNCTION(BlueprintCallable, Server, Reliable)
  void ServerSetPlayerName(const FString& NewName) const;
};
