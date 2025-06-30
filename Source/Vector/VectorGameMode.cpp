#include "VectorGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "VoxelWorld.h"


AActor* AVectorGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
    if (!bVoxelWorldSetupAttempted)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVoxelWorld::StaticClass(), FoundActors);

        if (FoundActors.Num() > 0)
        {
            AVoxelWorld* VoxelWorld = Cast<AVoxelWorld>(FoundActors[0]);
            if (VoxelWorld)
            {
                int32 NumDesiredPlayers = GetNumPlayers();
                if (NumDesiredPlayers == 0)
                {
                    NumDesiredPlayers = 1;
                    UE_LOG(LogTemp, Warning, TEXT("AVectorGameMode::FindPlayerStart_Implementation: No players currently. Requesting VoxelWorld to generate 1 player start."));
                }
                VoxelWorld->InitializeWorld(NumDesiredPlayers);
                UE_LOG(LogTemp, Log, TEXT("AVectorGameMode::FindPlayerStart_Implementation: Requested VoxelWorld initialization. Number of players: %d"), NumDesiredPlayers);

                AvailablePlayerStarts.Empty();
                AvailablePlayerStarts.Append(VoxelWorld->GetPlayerStartPoints());

                UE_LOG(LogTemp, Log, TEXT("AVectorGameMode::FindPlayerStart_Implementation: Directly retrieved %d PlayerStarts from VoxelWorld."), AvailablePlayerStarts.Num());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("AVectorGameMode::FindPlayerStart_Implementation: Found actor is not AVoxelWorld."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AVectorGameMode::FindPlayerStart_Implementation: AVoxelWorld not found in the level. PlayerStarts will not be dynamically assigned by VoxelWorld."));
        }
        bVoxelWorldSetupAttempted = true;
    }

    if (AvailablePlayerStarts.Num() > 0)
    {
        APlayerStart* ChosenPlayerStart = AvailablePlayerStarts[0];
        AvailablePlayerStarts.RemoveAt(0);

        UE_LOG(LogTemp, Log, TEXT("Player %s assigned to PlayerStart at %s."),
            *Player->GetName(),
            *ChosenPlayerStart->GetActorLocation().ToString());
        return ChosenPlayerStart;
    }

    UE_LOG(LogTemp, Warning, TEXT("No dynamic PlayerStarts available. Falling back to default FindPlayerStart behavior."));
    return Super::FindPlayerStart_Implementation(Player, IncomingName);
}