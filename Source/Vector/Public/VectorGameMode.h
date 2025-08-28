#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "VectorGameMode.generated.h"

class AVoxelWorldActor;

UCLASS()

class VECTOR_API AVectorGameMode : public AGameMode {
  GENERATED_BODY()

  virtual AActor* FindPlayerStart_Implementation(AController* Controller,
                                                 const FString& Name) override;
};
