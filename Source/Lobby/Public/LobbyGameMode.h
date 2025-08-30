#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

class AVectorPlayerState;
class ALobbyGameState;
struct FUniqueNetIdRepl;
class AActor;
class AController;

UCLASS()

class LOBBY_API ALobbyGameMode : public AGameModeBase {
  GENERATED_BODY()

 public:
  UFUNCTION(BlueprintCallable)
  void StartGame() const;

  virtual void PreLogin(const FString& Options, const FString& Address,
                        const FUniqueNetIdRepl& UniqueId,
                        FString& ErrorMessage) override;

  virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer,
                                                    AActor* StartSpot) override;

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