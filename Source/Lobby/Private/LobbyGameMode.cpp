#include "LobbyGameMode.h"
#include "LobbyGameState.h"
#include "VectorGameInstance.h"

void ALobbyGameMode::StartGame() const {
  if (GetWorld()->GetFirstLocalPlayerFromController()) {
    if (const TObjectPtr<UVectorGameInstance> GI =
            Cast<UVectorGameInstance>(GetGameInstance())) {
      if (const TObjectPtr<ALobbyGameState> GS =
              GetGameState<ALobbyGameState>()) {
        GI->TeamNames = GS->GetTeamNames();
      }
    }
  }

  GetWorld()->ServerTravel(TEXT("/Game/Vector/Maps/Vector?listen"));
}
