#include "VectorGameMode.h"

#include "Actors/VoxelWorld.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ObjectPtr.h"

AActor *
AVectorGameMode::FindPlayerStart_Implementation(AController *Player,
                                                const FString &IncomingName) {
  if (!bVoxelWorldSetupAttempted) {
    if (const TObjectPtr<AVoxelWorld> VoxelWorld =
            Cast<AVoxelWorld>(UGameplayStatics::GetActorOfClass(
                GetWorld(), AVoxelWorld::StaticClass()))) {
      const int32 NumDesiredPlayers = GetNumPlayers();
      VoxelWorld->Initialize(NumDesiredPlayers);

      AvailablePlayerStarts.Empty();
      AvailablePlayerStarts.Append(VoxelWorld->GetPlayerStarts());
    }
    bVoxelWorldSetupAttempted = true;
  }

  if (!AvailablePlayerStarts.IsEmpty()) {
    const TObjectPtr<APlayerStart> ChosenPlayerStart = AvailablePlayerStarts[0];
    AvailablePlayerStarts.RemoveAt(0);

    return ChosenPlayerStart;
  }

  UE_LOG(LogTemp, Warning,
         TEXT("No dynamic PlayerStarts available. Falling back to default "
              "FindPlayerStart behavior."));
  return Super::FindPlayerStart_Implementation(Player, IncomingName);
}