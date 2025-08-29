#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

class AVectorPlayerState;
class ALobbyGameMode;

UCLASS()

class LOBBY_API ALobbyPlayerController : public APlayerController {
  GENERATED_BODY()

  UPROPERTY()
  TObjectPtr<AVectorPlayerState> VectorPlayerState;

  virtual void BeginPlay() override;

 public:
  UFUNCTION(BlueprintCallable, Server, Reliable)
  void ServerAddTeam(const FString& Name, const FString& Password) const;

  UFUNCTION(BlueprintCallable, Server, Reliable)
  void ServerJoinTeam(const FString& Name, const FString& Password) const;

  UFUNCTION(BlueprintCallable, Server, Reliable)
  void ServerLeaveTeam() const;

  UFUNCTION(BlueprintCallable, Server, Reliable)
  void ServerSetPlayerName(const FString& Name) const;
};
