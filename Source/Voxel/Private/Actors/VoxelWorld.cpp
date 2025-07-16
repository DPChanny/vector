#include "Actors/VoxelWorld.h"

#include "DataAssets/VoxelBlockDataAsset.h"
#include "DataAssets/VoxelBorderDataAsset.h"
#include "DataAssets/VoxelVoidDataAsset.h"
#include "GameFramework/PlayerStart.h"
#include "Managers/BuildManager.h"
#include "Managers/DataManager.h"
#include "Managers/DebugManager.h"
#include "Managers/MeshManager.h"

AVoxelWorld::AVoxelWorld() { PrimaryActorTick.bCanEverTick = false; }

void AVoxelWorld::Initialize(const int32 NumberOfPlayers) {
  DebugManager = NewObject<UDebugManager>(this);
  DataManager = NewObject<UDataManager>(this);
  BuildManager = NewObject<UBuildManager>(this);
  MeshManager = NewObject<UMeshManager>(this);

  DebugManager->Initialize(VoxelDebugActor);
  DataManager->Initialize(WorldSizeInChunks, ChunkSize, VoxelSize,
                          VoxelChunkActor);
  BuildManager->Initialize();
  MeshManager->Initialize();

  for (int32 x = 0; x < WorldSizeInChunks.X; ++x) {
    for (int32 y = 0; y < WorldSizeInChunks.Y; ++y) {
      for (int32 z = 0; z < WorldSizeInChunks.Z; ++z) {
        DataManager->LoadChunk(FIntVector(x, y, z));
      }
    }
  }

  const FIntVector WorldSize = WorldSizeInChunks * ChunkSize;
  for (int32 x = 0; x < WorldSize.X; ++x) {
    for (int32 y = 0; y < WorldSize.Y; ++y) {
      for (int32 z = 0; z < WorldSize.Z; ++z) {
        const FIntVector VoxelCoord = FIntVector(x, y, z);

        if (!x || !y || !z || x == WorldSize.X - 1 || y == WorldSize.Y - 1 ||
            z == WorldSize.Z - 1) {
          DataManager->SetVoxelData(
              VoxelCoord, new FVoxelBorderData(VoxelBorderDataAsset), false);
        } else {
          DataManager->SetVoxelData(
              VoxelCoord,
              new FVoxelBlockData(VoxelDefaultBlockDataAsset,
                                  VoxelDefaultBlockDataAsset->MaxDurability),
              false);
        }
      }
    }
  }

  InitializeNexuses(NumberOfPlayers);

  MeshManager->FlushDirtyChunks();
}

void AVoxelWorld::InitializeNexuses(int32 NexusCount) {
  Nexuses.Empty();
  for (TObjectPtr StartPoint : PlayerStarts) {
    if (StartPoint) {
      StartPoint->Destroy();
    }
  }
  PlayerStarts.Empty();

  const FIntVector WorldSize = WorldSizeInChunks * ChunkSize;
  const FVector WorldMaxBounds = FVector(WorldSize) * VoxelSize;
  const FVector WorldMinBounds = FVector::ZeroVector;

  for (int32 i = 0; i < NexusCount; ++i) {
    constexpr int32 MaxPlacementAttempts = 100;
    FNexus NewRoom;
    NewRoom.Radius = NexusRadius;

    const FVector SafeMinBounds = WorldMinBounds + NewRoom.Radius;
    const FVector SafeMaxBounds = WorldMaxBounds - NewRoom.Radius;

    if (SafeMinBounds.X >= SafeMaxBounds.X ||
        SafeMinBounds.Y >= SafeMaxBounds.Y ||
        SafeMinBounds.Z >= SafeMaxBounds.Z) {
      UE_LOG(LogTemp, Warning,
             TEXT("Room radius is too large for the world size. Skipping room "
                  "generation for room %d."),
             i);
      continue;
    }

    bool bRoomPlaced = false;
    for (int32 Attempt = 0; Attempt < MaxPlacementAttempts; ++Attempt) {
      FVector CenterLocation =
          FVector(FMath::RandRange(SafeMinBounds.X, SafeMaxBounds.X),
                  FMath::RandRange(SafeMinBounds.Y, SafeMaxBounds.Y),
                  FMath::RandRange(SafeMinBounds.Z, SafeMaxBounds.Z));
      NewRoom.Center = CenterLocation;

      bool bOverlaps = false;
      for (const auto &[Center, Radius] : Nexuses) {
        if (FVector::Dist(NewRoom.Center, Center) < NewRoom.Radius + Radius) {
          bOverlaps = true;
          break;
        }
      }

      if (!bOverlaps) {
        Nexuses.Add(NewRoom);
        bRoomPlaced = true;
        break;
      }
    }

    if (!bRoomPlaced) {
      UE_LOG(LogTemp, Warning,
             TEXT("Failed to place room %d after %d attempts. Skipping."), i,
             MaxPlacementAttempts);
      continue;
    }

    TSet<FIntVector> GlobalCoordsInRadius;
    BuildManager->GetGlobalCoordsInRadius(
        DataManager->WorldToGlobalCoord(NewRoom.Center), NewRoom.Radius,
        GlobalCoordsInRadius);

    for (const FIntVector &GlobalCoord : GlobalCoordsInRadius) {
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
