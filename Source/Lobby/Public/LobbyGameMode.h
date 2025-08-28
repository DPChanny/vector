#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

UCLASS()

class LOBBY_API ALobbyGameMode : public AGameModeBase {
  GENERATED_BODY()

  virtual AActor* FindPlayerStart_Implementation(AController* Controller,
                                                 const FString& Name) override;

  virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole,
                                   const FString& Portal,
                                   const FString& Options,
                                   const FUniqueNetIdRepl& UniqueId,
                                   FString& ErrorMessage) override;
  virtual void PreLogin(const FString& Options, const FString& Address,
                        const FUniqueNetIdRepl& UniqueId,
                        FString& ErrorMessage) override;
  virtual void PostLogin(APlayerController* NewPlayer) override;

  virtual void RestartPlayer(AController* NewPlayer) override;
};
