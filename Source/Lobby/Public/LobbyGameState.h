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
  bool bUsePassword = false;

  UPROPERTY(Replicated, VisibleAnywhere)
  int32 MaxPlayersPerTeam;

  UPROPERTY(Replicated, VisibleAnywhere)
  int32 MaxTeams;

  UPROPERTY(VisibleAnywhere)
  TArray<FTeam> Teams;
};
