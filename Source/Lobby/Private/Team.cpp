#include "Team.h"
#include "LobbyGameState.h"
#include "Net/UnrealNetwork.h"
#include "VectorPlayerState.h"

void UTeam::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(UTeam, Name);
  DOREPLIFETIME(UTeam, Password);
  DOREPLIFETIME(UTeam, Leader);
  DOREPLIFETIME(UTeam, Members);
}

void UTeam::Join(const TObjectPtr<AVectorPlayerState> PlayerState,
                 const FString& InPassword) {
  if (!OwningGameState || !PlayerState || !OwningGameState->HasAuthority()) {
    return;
  }

  if (OwningGameState->bPasswordAllowed && !Password.IsEmpty() &&
      Password != InPassword) {
    return;
  }

  if (Members.Num() >= OwningGameState->MaxPlayersPerTeam) {
    return;
  }

  if (!Members.Contains(PlayerState)) {
    Members.Add(PlayerState);
    PlayerState->TeamName = Name;
  }
}

void UTeam::Leave(const TObjectPtr<AVectorPlayerState> PlayerState) {
  if (!OwningGameState || !PlayerState || !OwningGameState->HasAuthority()) {
    return;
  }

  if (Members.Contains(PlayerState)) {
    Members.Remove(PlayerState);
    PlayerState->TeamName.Empty();

    if (Members.IsEmpty()) {
      OwningGameState->RemoveTeam(Name);
    } else if (Leader == PlayerState) {
      Leader = Members[0];
    }
  }
}
