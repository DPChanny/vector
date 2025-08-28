#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "VectorGameInstance.generated.h"

UCLASS()

class VECTORCORE_API UVectorGameInstance : public UGameInstance {
  GENERATED_BODY()

  TArray<FString> TeamNames;
};
