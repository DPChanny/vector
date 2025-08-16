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

void AVoxelWorldActor::InitializeNexuses(
    const int32 NexusCount, TSet<TObjectPtr<APlayerStart>>& OutPlayerStart) {
  Nexuses.Empty();

  for (int32 i = 0; i < NexusCount; ++i) {
    const float Phi = acos(1 - 2 * (i + 0.5f) / NexusCount);
    constexpr float GoldenRatio = 1.618033988749895f;
    const float Theta = GoldenRatio * 2 * PI * i;

    FVector Dir(FMath::Sin(Phi) * FMath::Cos(Theta),
                FMath::Sin(Phi) * FMath::Sin(Theta), FMath::Cos(Phi));

    FNexus NewNexus(Dir * NexusPlacementRadius, NexusRadius);

    Nexuses.Add(NewNexus);

    TSet<FIntVector> GlobalCoordsInRadius;
    BuildManager->GetGlobalCoordsInRadius(
        DataManager->WorldToGlobalCoord(NewNexus.Center), NewNexus.Radius,
        GlobalCoordsInRadius);

    for (const FIntVector& GlobalCoord : GlobalCoordsInRadius) {
      DataManager->SetVoxelData(GlobalCoord, new FVoxelVoidData(), false);
    }

    if (TObjectPtr<APlayerStart> NewPlayerStart =
            GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass(),
                                                 NewNexus.Center,
                                                 FRotator::ZeroRotator)) {
      OutPlayerStart.Add(NewPlayerStart);
    }
  }

  MeshManager->FlushDirtyChunks();
}