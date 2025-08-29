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
  TObjectPtr<AVectorPlayerState> Leader;

  UPROPERTY(VisibleAnywhere)
  TArray<TObjectPtr<AVectorPlayerState>> Members;
};

UCLASS()

class LOBBY_API ALobbyGameState : public AGameStateBase {
  GENERATED_BODY()

 public:
  UPROPERTY(Replicated)
  TObjectPtr<AVectorPlayerState> LobbyOwner;

  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  UPROPERTY(Replicated, VisibleAnywhere)
  int32 MaxPlayersPerTeam;

  UPROPERTY(Replicated, VisibleAnywhere)
  int32 MaxTeams;

  UPROPERTY(VisibleAnywhere)
  TMap<FString, FTeam> Teams;

  UPROPERTY(Replicated, VisibleAnywhere)
  bool bPasswordAllowed = true;
  UPROPERTY(Replicated, VisibleAnywhere)
  bool bAddTeamAllowed = true;
  UPROPERTY(Replicated, VisibleAnywhere)
  bool bJoinTeamAllowed = true;

  UPROPERTY(Replicated, VisibleAnywhere)
  bool bUsePassword = true;

  void AddTeam(const FString& Name, const FString& Password,
               const TObjectPtr<AVectorPlayerState> VectorPlayerState);

  void JoinTeam(const FString& Name, const FString& Password,
                const TObjectPtr<AVectorPlayerState> VectorPlayerState);

  void LeaveTeam(const TObjectPtr<AVectorPlayerState> VectorPlayerState);
};
