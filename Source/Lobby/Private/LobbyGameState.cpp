#include "LobbyGameState.h"
#include "Net/UnrealNetwork.h"
#include "VectorPlayerState.h"

void ALobbyGameState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(ALobbyGameState, Teams);
  DOREPLIFETIME(ALobbyGameState, MaxPlayersPerTeam);
  DOREPLIFETIME(ALobbyGameState, MaxTeams);
  DOREPLIFETIME(ALobbyGameState, bPasswordAllowed);
}

void ALobbyGameState::JoinTeam(const int32 TeamID, const FString& Password,
                               AVectorPlayerState* PlayerState,
                               const bool bIsServer) {
  if (!bIsServer && !bJoinTeamAllowed) {
    return;
  }

  FTeam& Team = Teams[TeamID];
  if (bPasswordAllowed && Team.Password.Compare(Password)) {
    return;
  }

  if (Team.Members.Num() >= MaxPlayersPerTeam) {
    return;
  }

  if (!Team.Members.Contains(PlayerState)) {
    Team.Members.Add(PlayerState);
    PlayerState->TeamID = TeamID;
  }
}

void ALobbyGameState::AddTeam(const FString& TeamName, const FString& Password,
                              const bool bIsServer) {
  if (!bIsServer && !bAddTeamAllowed) {
    return;
  }

  if (Teams.Num() >= MaxTeams) {
    return;
  }

  for (const FTeam& Team : Teams) {
    if (Team.Name == TeamName) {
      return;
    }
  }

  FTeam NewTeam;
  NewTeam.Name = TeamName;
  NewTeam.Password = Password;
  Teams.Add(NewTeam);
}

TArray<FString> ALobbyGameState::GetTeamNames() const {
  TArray<FString> Names;
  for (const FTeam& Team : Teams) {
    Names.Add(Team.Name);
  }
  return Names;
}