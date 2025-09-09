#include "LobbyGameState.h"

#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Team.h"
#include "VectorPlayerState.h"

void ALobbyGameState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(ALobbyGameState, Teams);

  DOREPLIFETIME(ALobbyGameState, MaxPlayersPerTeam);
  DOREPLIFETIME(ALobbyGameState, MaxTeams);

  DOREPLIFETIME(ALobbyGameState, bPasswordAllowed);
  DOREPLIFETIME(ALobbyGameState, bJoinTeamAllowed);
  DOREPLIFETIME(ALobbyGameState, bAddTeamAllowed);

  DOREPLIFETIME(ALobbyGameState, LobbyOwner);
}

bool ALobbyGameState::ReplicateSubobjects(UActorChannel* Channel,
                                          FOutBunch* Bunch,
                                          FReplicationFlags* RepFlags) {
  bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

  for (const TPair<FString, TObjectPtr<UTeam>>& TeamPair : Teams) {
    if (TeamPair.Value) {
      bWroteSomething |=
          Channel->ReplicateSubobject(TeamPair.Value, *Bunch, *RepFlags);
    }
  }

  return bWroteSomething;
}

void ALobbyGameState::JoinTeam(
    const FString& Name, const FString& Password,
    const TObjectPtr<AVectorPlayerState> VectorPlayerState) {
  if (!HasAuthority() || !VectorPlayerState) {
    return;
  }

  if (!Teams.Contains(Name)) {
    return;
  }

  UTeam& Team = *Teams[Name];

  if (bPasswordAllowed && !Team.Password.IsEmpty() &&
      Team.Password != Password) {
    return;
  }

  if (Team.Members.Num() >= MaxPlayersPerTeam) {
    return;
  }

  LeaveTeam(VectorPlayerState);

  Team.Members.Add(VectorPlayerState);
  VectorPlayerState->TeamName = Name;
}

void ALobbyGameState::LeaveTeam(
    const TObjectPtr<AVectorPlayerState> VectorPlayerState) {
  if (!HasAuthority() || !VectorPlayerState) {
    return;
  }

  FString& Name = VectorPlayerState->TeamName;

  if (Name.IsEmpty()) {
    return;
  }

  if (Teams.Contains(Name)) {
    UTeam& Team = *Teams[Name];
    Team.Members.Remove(VectorPlayerState);

    if (Team.Members.IsEmpty()) {
      Teams.Remove(Name);
    } else if (Team.Leader == VectorPlayerState) {
      Team.Leader = Team.Members[0];
    }
  }

  Name.Empty();
}

void ALobbyGameState::OnTeamsChanged_Implementation() {}

bool ALobbyGameState::IsFull() const {
  return PlayerArray.Num() > MaxPlayersPerTeam * MaxPlayersPerTeam;
}

void ALobbyGameState::AddTeam(
    const FString& Name, const FString& Password,
    const TObjectPtr<AVectorPlayerState> VectorPlayerState) {
  if (!HasAuthority() || !VectorPlayerState) {
    return;
  }

  if (Teams.Num() >= MaxTeams) {
    return;
  }

  if (Teams.Contains(Name)) {
    return;
  }

  LeaveTeam(VectorPlayerState);

  // Teams.Emplace(Name, Name, Password, VectorPlayerState, {VectorPlayerState});

  VectorPlayerState->TeamName = Name;
}