#include "VectorGameMode.h"

#include "Actors/VoxelWorldActor.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

AActor* AVectorGameMode::FindPlayerStart_Implementation(AController* Controller,
                                                        const FString& Name) {
  if (!bNexusInitialized) {
    if (const TObjectPtr<AVoxelWorldActor> VoxelWorld =
            Cast<AVoxelWorldActor>(UGameplayStatics::GetActorOfClass(
                GetWorld(), AVoxelWorldActor::StaticClass()))) {
      const int32 NumDesiredPlayers = GetNumPlayers();
      VoxelWorld->InitializeNexuses(NumDesiredPlayers, AvailablePlayerStarts);
    }

    bNexusInitialized = true;
  }

  if (!AvailablePlayerStarts.IsEmpty()) {
    const TObjectPtr<APlayerStart> ChosenPlayerStart =
        *AvailablePlayerStarts.begin();
    AvailablePlayerStarts.Remove(ChosenPlayerStart);

    return ChosenPlayerStart;
  }

  UE_LOG(LogTemp, Warning,
         TEXT("No dynamic PlayerStarts available. Falling back to default "
              "FindPlayerStart behavior."));
  return Super::FindPlayerStart_Implementation(Controller, Name);
}