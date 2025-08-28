#include "LobbyPlayerController.h"
#include "LobbyGameState.h"
#include "VectorPlayerState.h"

void ALobbyPlayerController::BeginPlay() {
  Super::BeginPlay();

  LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>();
  VectorPlayerState = GetPlayerState<AVectorPlayerState>();
}

void ALobbyPlayerController::ServerAddTeam_Implementation(
    const FString& TeamName, const FString& Password) const {
  if (LobbyGameState) {
    LobbyGameState->AddTeam(TeamName, Password);
  }
}

void ALobbyPlayerController::ServerJoinTeam_Implementation(
    const int32& TeamID, const FString& Password) const {
  if (LobbyGameState) {
    LobbyGameState->JoinTeam(TeamID, Password, VectorPlayerState);
  }
}

void ALobbyPlayerController::ServerSetPlayerName_Implementation(
    const FString& NewName) const {
  if (VectorPlayerState) {
    VectorPlayerState->SetPlayerName(NewName);
  }
}
