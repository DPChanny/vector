#include "LobbyGameMode.h"
#include "LobbyGameState.h"
#include "VectorPlayerState.h"

void ALobbyGameMode::StartGame() const {
  const TObjectPtr<ALobbyGameState> LobbyGameState =
      GetGameState<ALobbyGameState>();

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
  }

  GetWorld()->ServerTravel(TEXT("/Game/Vector/Maps/Vector"));
}

void ALobbyGameMode::PreLogin(const FString& Options, const FString& Address,
                              const FUniqueNetIdRepl& UniqueId,
                              FString& ErrorMessage) {
  const TObjectPtr<ALobbyGameState> LobbyGameState =
      GetGameState<ALobbyGameState>();

  if (!LobbyGameState) {
    ErrorMessage = "No Game State";
    return;
  }

  Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
  if (!ErrorMessage.IsEmpty()) {
    return;
  }

  if (LobbyGameState->IsFull()) {
    ErrorMessage = "Lobby Is Full";
  }
}

APawn* ALobbyGameMode::SpawnDefaultPawnFor_Implementation(
    AController* NewPlayer, AActor* StartSpot) {
  return nullptr;
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer) {
  Super::PostLogin(NewPlayer);

  const TObjectPtr<ALobbyGameState> LobbyGameState =
      GetGameState<ALobbyGameState>();

  if (!LobbyGameState) {
    return;
  }

  if (!LobbyGameState->LobbyOwner) {
    LobbyGameState->LobbyOwner =
        NewPlayer->GetPlayerState<AVectorPlayerState>();
  }
}

void ALobbyGameMode::HandleAddTeamRequest(
    const FString& Name, const FString& Password,
    const TObjectPtr<AVectorPlayerState> VectorPlayerState) const {
  if (!VectorPlayerState) {
    return;
  }

  const TObjectPtr<ALobbyGameState> LobbyGameState =
      GetGameState<ALobbyGameState>();

  if (!LobbyGameState) {
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
  if (!VectorPlayerState) {
    return;
  }

  const TObjectPtr<ALobbyGameState> LobbyGameState =
      GetGameState<ALobbyGameState>();

  if (!LobbyGameState) {
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
  if (!VectorPlayerState) {
    return;
  }

  if (const TObjectPtr<ALobbyGameState> LobbyGameState =
          GetGameState<ALobbyGameState>()) {
    LobbyGameState->LeaveTeam(VectorPlayerState);
  }
}
