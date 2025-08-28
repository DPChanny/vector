#include "LobbyGameMode.h"

AActor* ALobbyGameMode::FindPlayerStart_Implementation(AController* Controller,
                                                       const FString& Name) {
  return Super::FindPlayerStart_Implementation(Controller, Name);
}

APlayerController* ALobbyGameMode::Login(UPlayer* NewPlayer,
                                         ENetRole InRemoteRole,
                                         const FString& Portal,
                                         const FString& Options,
                                         const FUniqueNetIdRepl& UniqueId,
                                         FString& ErrorMessage) {
  return Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId,
                      ErrorMessage);
}

void ALobbyGameMode::PreLogin(const FString& Options, const FString& Address,
                              const FUniqueNetIdRepl& UniqueId,
                              FString& ErrorMessage) {
  Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer) {
  Super::PostLogin(NewPlayer);
}

void ALobbyGameMode::RestartPlayer(AController* NewPlayer) {
  Super::RestartPlayer(NewPlayer);
}