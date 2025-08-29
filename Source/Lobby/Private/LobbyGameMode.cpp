#include "LobbyGameMode.h"
#include "LobbyGameState.h"
#include "VectorPlayerState.h"

void ALobbyGameMode::BeginPlay() {
  Super::BeginPlay();

  LobbyGameState = GetGameState<ALobbyGameState>();
}

void ALobbyGameMode::StartGame() const {
  GetWorld()->ServerTravel(TEXT("/Game/Vector/Maps/Vector?listen"));
}

void ALobbyGameMode::HandleAddTeamRequest(
    const FString& Name, const FString& Password,
    const TObjectPtr<const APlayerController> PlayerController) const {
  if (!LobbyGameState->bAddTeamAllowed &&
      !PlayerController->IsLocalController()) {
    return;
  }

  LobbyGameState->AddTeam(
      Name, Password, PlayerController->GetPlayerState<AVectorPlayerState>());
}

void ALobbyGameMode::HandleJoinTeamRequest(
    const FString& Name, const FString& Password,
    const TObjectPtr<const APlayerController> PlayerController) const {
  if (!LobbyGameState->bJoinTeamAllowed &&
      !PlayerController->IsLocalController()) {
    return;
  }

  LobbyGameState->JoinTeam(
      Name, Password, PlayerController->GetPlayerState<AVectorPlayerState>());
}

void ALobbyGameMode::HandleLeaveTeamRequest(
    const TObjectPtr<const APlayerController> PlayerController) const {
  LobbyGameState->LeaveTeam(
      PlayerController->GetPlayerState<AVectorPlayerState>());
}
