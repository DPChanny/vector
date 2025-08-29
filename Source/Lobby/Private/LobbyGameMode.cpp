#include "LobbyGameMode.h"
#include "LobbyGameState.h"
#include "VectorPlayerState.h"

void ALobbyGameMode::BeginPlay() {
  Super::BeginPlay();

  LobbyGameState = GetGameState<ALobbyGameState>();
}

void ALobbyGameMode::StartGame() const {
  if (!LobbyGameState) {
    return;
  }

  for (TObjectPtr PlayerState : LobbyGameState->PlayerArray) {
    if (const TObjectPtr<AVectorPlayerState> VectorPlayerState =
            Cast<AVectorPlayerState>(PlayerState)) {
      if (VectorPlayerState->TeamName.IsEmpty()) {
        return;
      }
    }

    GetWorld()->ServerTravel(TEXT("/Game/Vector/Maps/Vector"));
  }
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer) {
  Super::PostLogin(NewPlayer);

  if (LobbyGameState && !LobbyGameState->LobbyOwner) {
    LobbyGameState->LobbyOwner =
        NewPlayer->GetPlayerState<AVectorPlayerState>();
  }
}

void ALobbyGameMode::HandleAddTeamRequest(
    const FString& Name, const FString& Password,
    const TObjectPtr<AVectorPlayerState> VectorPlayerState) const {
  if (!VectorPlayerState || !LobbyGameState) {
    return;
  }

  if (!LobbyGameState->bAddTeamAllowed &&
      VectorPlayerState != LobbyGameState->LobbyOwner) {
    return;
  }

  LobbyGameState->AddTeam(Name, Password, VectorPlayerState);
}

void ALobbyGameMode::HandleJoinTeamRequest(
    const FString& Name, const FString& Password,
    const TObjectPtr<AVectorPlayerState> VectorPlayerState) const {
  if (!VectorPlayerState || !LobbyGameState) {
    return;
  }

  if (!LobbyGameState->bJoinTeamAllowed &&
      VectorPlayerState != LobbyGameState->LobbyOwner) {
    return;
  }

  LobbyGameState->JoinTeam(Name, Password, VectorPlayerState);
}

void ALobbyGameMode::HandleLeaveTeamRequest(
    const TObjectPtr<AVectorPlayerState> VectorPlayerState) const {
  if (!VectorPlayerState || !LobbyGameState) {
    return;
  }

  LobbyGameState->LeaveTeam(VectorPlayerState);
}
