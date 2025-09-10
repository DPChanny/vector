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
    const TObjectPtr<AVectorPlayerState> VectorPlayerState) const {
  if (!HasAuthority() || !VectorPlayerState) {
    return;
  }

  if (!VectorPlayerState->TeamName.IsEmpty() &&
      VectorPlayerState->TeamName != Name) {
    LeaveTeam(VectorPlayerState);
  }

  if (const TObjectPtr<UTeam> Team = Teams.FindRef(Name)) {
    Team->Join(VectorPlayerState, Password);
  }
}

void ALobbyGameState::LeaveTeam(
    const TObjectPtr<AVectorPlayerState> VectorPlayerState) const {
  if (!HasAuthority() || !VectorPlayerState ||
      VectorPlayerState->TeamName.IsEmpty()) {
    return;
  }

  if (const TObjectPtr<UTeam> Team =
          Teams.FindRef(VectorPlayerState->TeamName)) {
    Team->Leave(VectorPlayerState);
  }
}

void ALobbyGameState::RemoveTeam(const FString& TeamName) {
  if (HasAuthority()) {
    Teams.Remove(TeamName);
  }
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

  if (Teams.Num() >= MaxTeams || Teams.Contains(Name)) {
    return;
  }

  if (!VectorPlayerState->TeamName.IsEmpty()) {
    LeaveTeam(VectorPlayerState);
  }

  const TObjectPtr<UTeam> NewTeam = NewObject<UTeam>(this);
  NewTeam->Name = Name;
  NewTeam->Password = Password;
  NewTeam->Leader = VectorPlayerState;
  NewTeam->OwningGameState = this;

  Teams.Add(Name, NewTeam);

  NewTeam->Join(VectorPlayerState, Password);
}