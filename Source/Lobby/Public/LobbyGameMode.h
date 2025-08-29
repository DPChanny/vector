#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

class AVectorPlayerState;
class ALobbyGameState;

UCLASS()

class LOBBY_API ALobbyGameMode : public AGameModeBase {
  GENERATED_BODY()

  virtual void BeginPlay() override;

  UPROPERTY()
  TObjectPtr<ALobbyGameState> LobbyGameState;

 public:
  UFUNCTION(BlueprintCallable)
  void StartGame() const;

  virtual void PostLogin(APlayerController* NewPlayer) override;

  void HandleAddTeamRequest(
      const FString& Name, const FString& Password,
      TObjectPtr<AVectorPlayerState> VectorPlayerState) const;

  void HandleJoinTeamRequest(
      const FString& Name, const FString& Password,
      TObjectPtr<AVectorPlayerState> VectorPlayerState) const;

  void HandleLeaveTeamRequest(
      TObjectPtr<AVectorPlayerState> VectorPlayerState) const;
};