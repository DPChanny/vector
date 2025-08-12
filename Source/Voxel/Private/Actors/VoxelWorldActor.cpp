#include "Actors/VoxelWorldActor.h"

#include "DataAssets/VoxelVoidDataAsset.h"
#include "GameFramework/PlayerStart.h"
#include "Managers/BuildManager.h"
#include "Managers/DataManager.h"
#include "Managers/DebugManager.h"
#include "Managers/EntityManager.h"
#include "Managers/MeshManager.h"

AVoxelWorldActor::AVoxelWorldActor() {
  PrimaryActorTick.bCanEverTick = true;

  DebugManager = CreateDefaultSubobject<UDebugManager>(TEXT("DebugManager"));
  DataManager = CreateDefaultSubobject<UDataManager>(TEXT("DataManager"));
  BuildManager = CreateDefaultSubobject<UBuildManager>(TEXT("BuildManager"));
  MeshManager = CreateDefaultSubobject<UMeshManager>(TEXT("MeshManager"));
  EntityManager = CreateDefaultSubobject<UEntityManager>(TEXT("EntityManager"));
}

void AVoxelWorldActor::Initialize(const int32 NumberOfPlayers) {
  InitializeNexuses(NumberOfPlayers);

  MeshManager->FlushDirtyChunks();
}

void AVoxelWorldActor::InitializeNexuses(const int32 NexusCount) {
  Nexuses.Empty();
  for (TObjectPtr StartPoint : PlayerStarts) {
    if (StartPoint) {
      StartPoint->Destroy();
    }
  }
  PlayerStarts.Empty();

  const FVector Center = FVector::ZeroVector;
  const float RandomJitter = NexusPlacementRadius * 0.08f;

  for (int32 i = 0; i < NexusCount; ++i) {
    FNexus NewRoom;
    NewRoom.Radius = NexusRadius;

    const float Phi = acos(1 - 2 * (i + 0.5f) / NexusCount);
    constexpr float GoldenRatio = 1.618033988749895f;
    const float Theta = GoldenRatio * 2 * PI * i;

    FVector Dir;
    Dir.X = FMath::Sin(Phi) * FMath::Cos(Theta);
    Dir.Y = FMath::Sin(Phi) * FMath::Sin(Theta);
    Dir.Z = FMath::Cos(Phi);

    FVector Jitter(FMath::FRandRange(-RandomJitter, RandomJitter),
                   FMath::FRandRange(-RandomJitter, RandomJitter),
                   FMath::FRandRange(-RandomJitter, RandomJitter));

    NewRoom.Center = Center + Dir * NexusPlacementRadius + Jitter;

    Nexuses.Add(NewRoom);

    TSet<FIntVector> GlobalCoordsInRadius;
    BuildManager->GetGlobalCoordsInRadius(
        DataManager->WorldToGlobalCoord(NewRoom.Center), NewRoom.Radius,
        GlobalCoordsInRadius);

    for (const FIntVector& GlobalCoord : GlobalCoordsInRadius) {
      DataManager->SetVoxelData(GlobalCoord, new FVoxelVoidData(), false);
    }

    FTransform SpawnTransform(FRotator::ZeroRotator, NewRoom.Center);
    TObjectPtr<APlayerStart> NewPlayerStart =
        GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass(),
                                             SpawnTransform);
    if (NewPlayerStart) {
      PlayerStarts.Add(NewPlayerStart);
    }
  }
}