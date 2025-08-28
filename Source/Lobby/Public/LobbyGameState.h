#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LobbyGameState.generated.h"

class AVectorPlayerState;

USTRUCT()

struct FTeam {
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere)
  FString Name;

  UPROPERTY(VisibleAnywhere)
  FString Password;

  UPROPERTY(VisibleAnywhere)
  TArray<TObjectPtr<AVectorPlayerState>> Members;
};

UCLASS()

class LOBBY_API ALobbyGameState : public AGameStateBase {
  GENERATED_BODY()

 public:
  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  UPROPERTY(Replicated, VisibleAnywhere)
  int32 MaxPlayersPerTeam;

  UPROPERTY(Replicated, VisibleAnywhere)
  int32 MaxTeams;

  UPROPERTY(Replicated, VisibleAnywhere)
  TArray<FTeam> Teams;

  UPROPERTY(Replicated, VisibleAnywhere)
  bool bPasswordAllowed = true;
  UPROPERTY(Replicated, VisibleAnywhere)
  bool bAddTeamAllowed = true;
  UPROPERTY(Replicated, VisibleAnywhere)
  bool bJoinTeamAllowed = true;

  UPROPERTY(Replicated, VisibleAnywhere)
  bool bUsePassword = true;

  void AddTeam(const FString& TeamName, const FString& Password,
               const bool bIsServer = false);

  void JoinTeam(int32 TeamID, const FString& Password,
                AVectorPlayerState* PlayerState, const bool bIsServer = false);

  UFUNCTION(BlueprintCallable)
  TArray<FString> GetTeamNames() const;
};
