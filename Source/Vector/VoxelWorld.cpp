#include "VoxelWorld.h"

#include "GameFramework/PlayerStart.h"
#include "VoxelBlockDataAsset.h"
#include "VoxelBuild.h"
#include "VoxelData.h"
#include "VoxelDebug.h"
#include "VoxelMesh.h"

AVoxelWorld::AVoxelWorld() { PrimaryActorTick.bCanEverTick = false; }

void AVoxelWorld::Initialize(const int32 NumberOfPlayers) {
  VoxelDebug = NewObject<UVoxelDebug>(this);
  VoxelData = NewObject<UVoxelData>(this);
  VoxelBuild = NewObject<UVoxelBuild>(this);
  VoxelMesh = NewObject<UVoxelMesh>(this);

  VoxelDebug->Initialize(VoxelDebugActor);
  VoxelData->Initialize(WorldSizeInChunks, ChunkSize, VoxelSize, VoxelChunk,
                        VoxelBlockDataAssets, VoxelVoidDataAsset,
                        VoxelBorderDataAsset);
  VoxelBuild->Initialize();
  VoxelMesh->Initialize();

  float DefaultBlockMaxDurability;
  const UVoxelBlockDataAsset *DefaultBlockData = Cast<UVoxelBlockDataAsset>(
      VoxelData->GetVoxelDataAsset(UVoxelData::GetDefaultBlockID()));
  if (DefaultBlockData) {
    DefaultBlockMaxDurability = DefaultBlockData->MaxDurability;
  } else {
    DefaultBlockMaxDurability = 100.f;
  }

  for (int32 x = 0; x < WorldSizeInChunks.X; ++x) {
    for (int32 y = 0; y < WorldSizeInChunks.Y; ++y) {
      for (int32 z = 0; z < WorldSizeInChunks.Z; ++z) {
        VoxelData->LoadChunk(FIntVector(x, y, z));
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
          VoxelData->SetVoxel(VoxelCoord,
                              FVoxel(UVoxelData::GetBorderID(), 0.f), false);
        } else {
          VoxelData->SetVoxel(VoxelCoord,
                              FVoxel(UVoxelData::GetDefaultBlockID(),
                                     DefaultBlockMaxDurability),
                              false);
        }
      }
    }
  }

  InitializeNexuses(NumberOfPlayers);

  VoxelMesh->FlushDirtyChunks();
}

void AVoxelWorld::InitializeNexuses(int32 NexusCount) {
  Nexuses.Empty();
  for (APlayerStart *StartPoint : PlayerStarts) {
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
      for (const FNexus &ExistingRoom : Nexuses) {
        float DistanceBetweenCenters =
            FVector::Dist(NewRoom.Center, ExistingRoom.Center);
        if (DistanceBetweenCenters < (NewRoom.Radius + ExistingRoom.Radius)) {
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
    VoxelBuild->GetGlobalCoordsInRadius(NewRoom.Center, NewRoom.Radius,
                                        GlobalCoordsInRadius);

    for (const FIntVector &GlobalCoord : GlobalCoordsInRadius) {
      VoxelData->SetVoxel(GlobalCoord, FVoxel(UVoxelData::GetVoidID(), 0.f),
                          false);
    }

    FTransform SpawnTransform(FRotator::ZeroRotator, NewRoom.Center);
    APlayerStart *NewPlayerStart = GetWorld()->SpawnActor<APlayerStart>(
        APlayerStart::StaticClass(), SpawnTransform);
    if (NewPlayerStart) {
      PlayerStarts.Add(NewPlayerStart);
    }
  }
}
