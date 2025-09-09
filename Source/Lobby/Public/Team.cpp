#include "Team.h"
#include "Net/UnrealNetwork.h"

void UTeam::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const {
  UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(UTeam, Name);
  DOREPLIFETIME(UTeam, Password);
  DOREPLIFETIME(UTeam, Leader);
  DOREPLIFETIME(UTeam, Members);
}