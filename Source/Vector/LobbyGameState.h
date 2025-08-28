#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LobbyGameState.generated.h"

USTRUCT()

struct FTeamInfo {
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere)
  FString Name;

  UPROPERTY(VisibleAnywhere)
  FString Password;

  UPROPERTY(VisibleAnywhere)
  TArray<APlayerState> Members;
};

UCLASS()
class ALobbyGameState : public AGameStateBase {
  GENERATED_BODY()

public:
  UPROPERTY(Replicated, VisibleAnywhere)
  bool bUsePassword = false;

  UPROPERTY(Replicated, VisibleAnywhere)
  int32 MaxPlayersPerTeam;

  UPROPERTY(Replicated, VisibleAnywhere)
  int32 MaxTeams;

  UPROPERTY(VisibleAnywhere)
  TArray<FTeamInfo> Teams;
};
