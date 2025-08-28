#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "LobbyGameState.h"
#include "VectorGameInstance.generated.h"

UCLASS()
class UVectorGameInstance : public UGameInstance{
  GENERATED_BODY()

  TArray<FTeamInfo> LobbyTeamInfos;
};
