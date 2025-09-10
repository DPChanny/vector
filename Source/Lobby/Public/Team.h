#pragma once

#include "CoreMinimal.h"
#include "Team.generated.h"

class AVectorPlayerState;
class ALobbyGameState;

UCLASS()

class LOBBY_API UTeam : public UObject {
  GENERATED_BODY()

 public:
  virtual bool IsSupportedForNetworking() const override { return true; }

  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  void Join(TObjectPtr<AVectorPlayerState> PlayerState,
            const FString& InPassword);
  void Leave(TObjectPtr<AVectorPlayerState> PlayerState);

  UPROPERTY(VisibleAnywhere, Replicated)
  FString Name;

  UPROPERTY(VisibleAnywhere, Replicated)
  FString Password;

  UPROPERTY(VisibleAnywhere, Replicated)
  TObjectPtr<AVectorPlayerState> Leader;

  UPROPERTY(VisibleAnywhere, Replicated)
  TArray<TObjectPtr<AVectorPlayerState>> Members;

  UPROPERTY()
  TObjectPtr<ALobbyGameState> OwningGameState;
};
