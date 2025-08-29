#include "LobbyPlayerController.h"
#include "LobbyGameMode.h"
#include "VectorPlayerState.h"

void ALobbyPlayerController::BeginPlay() {
  Super::BeginPlay();

  VectorPlayerState = GetPlayerState<AVectorPlayerState>();
}

void ALobbyPlayerController::ServerAddTeam_Implementation(
    const FString& Name, const FString& Password) const {
  if (const TObjectPtr<ALobbyGameMode> LobbyGameMode =
          GetWorld()->GetAuthGameMode<ALobbyGameMode>()) {
    LobbyGameMode->HandleAddTeamRequest(Name, Password, this);
  }
}

void ALobbyPlayerController::ServerJoinTeam_Implementation(
    const FString& Name, const FString& Password) const {
  if (const TObjectPtr<ALobbyGameMode> LobbyGameMode =
          GetWorld()->GetAuthGameMode<ALobbyGameMode>()) {
    LobbyGameMode->HandleJoinTeamRequest(Name, Password, this);
  }
}

void ALobbyPlayerController::ServerLeaveTeam_Implementation() const {
  if (const TObjectPtr<ALobbyGameMode> LobbyGameMode =
          GetWorld()->GetAuthGameMode<ALobbyGameMode>()) {
    LobbyGameMode->HandleLeaveTeamRequest(this);
  }
}

void ALobbyPlayerController::ServerSetPlayerName_Implementation(
    const FString& Name) const {
  VectorPlayerState->SetPlayerName(Name);
}
