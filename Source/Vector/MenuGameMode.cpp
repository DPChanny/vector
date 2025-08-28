#include "MenuGameMode.h"

AActor* AMenuGameMode::FindPlayerStart_Implementation(AController* Controller,
                                                      const FString& Name) {
  return Super::FindPlayerStart_Implementation(Controller, Name);
}

APlayerController* AMenuGameMode::Login(UPlayer* NewPlayer,
                                        const ENetRole InRemoteRole,
                                        const FString& Portal,
                                        const FString& Options,
                                        const FUniqueNetIdRepl& UniqueId,
                                        FString& ErrorMessage) {
  return Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId,
                      ErrorMessage);
}

void AMenuGameMode::PreLogin(const FString& Options, const FString& Address,
                             const FUniqueNetIdRepl& UniqueId,
                             FString& ErrorMessage) {
  Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

void AMenuGameMode::PostLogin(APlayerController* NewPlayer) {
  Super::PostLogin(NewPlayer);
}

void AMenuGameMode::RestartPlayer(AController* NewPlayer) {
  Super::RestartPlayer(NewPlayer);
}