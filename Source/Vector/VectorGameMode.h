#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "VectorGameMode.generated.h"

UCLASS()
class VECTOR_API AVectorGameMode : public AGameModeBase {
  GENERATED_BODY()

 protected:
  virtual AActor* FindPlayerStart_Implementation(
      AController* Player, const FString& IncomingName) override;

  UPROPERTY()
  TArray<APlayerStart*> AvailablePlayerStarts;

  bool bVoxelWorldSetupAttempted = false;
};
