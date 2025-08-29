#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

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

  void HandleAddTeamRequest(
      const FString& Name, const FString& Password,
      const TObjectPtr<const APlayerController> PlayerController) const;

  void HandleJoinTeamRequest(
      const FString& Name, const FString& Password,
      const TObjectPtr<const APlayerController> PlayerController) const;

  void HandleLeaveTeamRequest(
      const TObjectPtr<const APlayerController> PlayerController) const;
};