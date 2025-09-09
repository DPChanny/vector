#pragma once

#include "CoreMinimal.h"
#include "Team.generated.h"

class AVectorPlayerState;

UCLASS()

class UTeam : public UObject {
  GENERATED_BODY()

  virtual bool IsSupportedForNetworking() const override { return true; }

  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty>& OutLifetimeProps) const override;

 public:
  UPROPERTY(VisibleAnywhere, Replicated)
  FString Name;

  UPROPERTY(VisibleAnywhere, Replicated)
  FString Password;

  UPROPERTY(VisibleAnywhere, Replicated)
  TObjectPtr<AVectorPlayerState> Leader;

  UPROPERTY(VisibleAnywhere, Replicated)
  TArray<TObjectPtr<AVectorPlayerState>> Members;
};
