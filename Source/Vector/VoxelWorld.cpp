#include "VoxelWorld.h"

#include "GameFramework/PlayerStart.h"
#include "VoxelBlockDataAsset.h"
#include "VoxelData.h"
#include "VoxelDebug.h"

AVoxelWorld::AVoxelWorld() { PrimaryActorTick.bCanEverTick = false; }

void AVoxelWorld::Initialize(const int32 NumberOfPlayers) {
  VoxelDebug = NewObject<UVoxelDebug>(this);
  VoxelData = NewObject<UVoxelData>(this);

  VoxelDebug->Initialize(VoxelDebugActor);
  VoxelData->Initialize(WorldSizeInChunks, ChunkSize, VoxelSize, VoxelChunk,
                        VoxelBlockDataAssets, VoxelVoidDataAsset,
                        VoxelBorderDataAsset);

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

  for (int32 x = 0; x < WorldSizeInChunks.X; ++x) {
    for (int32 y = 0; y < WorldSizeInChunks.Y; ++y) {
      for (int32 z = 0; z < WorldSizeInChunks.Z; ++z) {
        if (const FChunk *Chunk = VoxelData->GetChunk(FIntVector(x, y, z))) {
          Chunk->UpdateMesh();
        }
      }
    }
  }
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

    FIntVector MinAffectedGrid =
        FIntVector(FVector(NewRoom.Center - NewRoom.Radius) / VoxelSize);
    FIntVector MaxAffectedGrid =
        FIntVector(FVector(NewRoom.Center + NewRoom.Radius) / VoxelSize);

    for (int32 z = FMath::Max(0, MinAffectedGrid.Z);
         z < FMath::Min(WorldSize.Z, MaxAffectedGrid.Z + 1); ++z) {
      for (int32 y = FMath::Max(0, MinAffectedGrid.Y);
           y < FMath::Min(WorldSize.Y, MaxAffectedGrid.Y + 1); ++y) {
        for (int32 x = FMath::Max(0, MinAffectedGrid.X);
             x < FMath::Min(WorldSize.X, MaxAffectedGrid.X + 1); ++x) {
          const FIntVector GlobalCoord(x, y, z);
          const FVector WorldCoord = FVector(GlobalCoord) * VoxelSize;

          if (FVector::DistSquared(WorldCoord, NewRoom.Center) <
              FMath::Square(NewRoom.Radius)) {
            VoxelData->SetVoxel(GlobalCoord,
                                FVoxel(UVoxelData::GetVoidID(), 0.f), false);
          }
        }
      }
    }

    FTransform SpawnTransform(FRotator::ZeroRotator, NewRoom.Center);
    APlayerStart *NewPlayerStart = GetWorld()->SpawnActor<APlayerStart>(
        APlayerStart::StaticClass(), SpawnTransform);
    if (NewPlayerStart) {
      PlayerStarts.Add(NewPlayerStart);
      UE_LOG(LogTemp, Log, TEXT("Generated PlayerStart at: %s"),
             *NewPlayerStart->GetActorLocation().ToString());
    } else {
      UE_LOG(LogTemp, Error,
             TEXT("Failed to spawn PlayerStart for room %d at %s"), i,
             *NewRoom.Center.ToString());
    }
  }
  UE_LOG(LogTemp, Log, TEXT("VoxelWorld: Generated %d PlayerStarts."),
         PlayerStarts.Num());
}

void AVoxelWorld::DamageVoxel(const FVector &Center, const float Radius,
                              const float DamageAmount) const {
  auto DamageLogic = [&](const FIntVector &VoxelCoord,
                         TSet<FIntVector> &DirtyChunks) {
    if (!VoxelData) {
      return;
    }

    if (!Cast<UVoxelBlockDataAsset>(
            VoxelData->GetVoxelDataAsset(VoxelData->GetVoxelID(VoxelCoord)))) {
      return;
    }

    const float NewDurability =
        VoxelData->GetDurability(VoxelCoord) - DamageAmount;

    if (NewDurability <= 0) {
      VoxelData->SetVoxel(VoxelCoord, FVoxel(UVoxelData::GetVoidID(), 0.f));
    } else {
      VoxelData->SetDurability(VoxelCoord, NewDurability);
    }

    AddDirtyChunk(VoxelCoord, DirtyChunks);
  };

  ProcessVoxel(Center, Radius, DamageLogic);
}

void AVoxelWorld::ConstructVoxel(const FVector &Center, const float Radius,
                                 const float ConstructionAmount,
                                 const int32 VoxelIDToConstruct) const {
  auto ConstructLogic = [&](const FIntVector &GlobalCoord,
                            TSet<FIntVector> &DirtyChunks) {
    if (!VoxelData) {
      return;
    }

    const UVoxelBlockDataAsset *VoxelDataAsset = Cast<UVoxelBlockDataAsset>(
        VoxelData->GetVoxelDataAsset(VoxelData->GetVoxelID(GlobalCoord)));
    if (!VoxelDataAsset) {
      return;
    }

    const UVoxelBlockDataAsset *NewVoxelData = Cast<UVoxelBlockDataAsset>(
        VoxelData->GetVoxelDataAsset(VoxelIDToConstruct));
    if (!NewVoxelData) {
      return;
    }

    const float NewDurability =
        VoxelData->GetDurability(GlobalCoord) + ConstructionAmount;

    if (NewDurability >= VoxelDataAsset->MaxDurability) {
      VoxelData->SetDurability(GlobalCoord, VoxelDataAsset->MaxDurability);

      const TArray NeighborOffsets = {
          FIntVector(1, 0, 0),  FIntVector(-1, 0, 0), FIntVector(0, 1, 0),
          FIntVector(0, -1, 0), FIntVector(0, 0, 1),  FIntVector(0, 0, -1)};

      for (const FIntVector &NeighborOffset : NeighborOffsets) {
        const FIntVector NeighborGlobalCoord = GlobalCoord + NeighborOffset;
        if (VoxelData->GetVoxelID(NeighborGlobalCoord) ==
            UVoxelData::GetVoidID()) {
          VoxelData->SetVoxel(NeighborGlobalCoord,
                              FVoxel(VoxelIDToConstruct, 0.f));

          for (const FIntVector &CheckOffset : NeighborOffsets) {
            const FIntVector CheckGlobalCoord =
                NeighborGlobalCoord + CheckOffset;

            if (CheckGlobalCoord == GlobalCoord) {
              continue;
            }

            if (const UVoxelBlockDataAsset *EnclosedBlockData =
                    Cast<UVoxelBlockDataAsset>(VoxelData->GetVoxelDataAsset(
                        VoxelData->GetVoxelID(CheckGlobalCoord)))) {
              if (!IsSurfaceVoxel(CheckGlobalCoord)) {
                VoxelData->SetDurability(CheckGlobalCoord,
                                         EnclosedBlockData->MaxDurability);
                AddDirtyChunk(CheckGlobalCoord, DirtyChunks);
              }
            }
          }

          if (!IsSurfaceVoxel(NeighborGlobalCoord)) {
            VoxelData->SetDurability(NeighborGlobalCoord,
                                     NewVoxelData->MaxDurability);
          }

          AddDirtyChunk(NeighborGlobalCoord, DirtyChunks);
        }
      }
    } else {
      VoxelData->SetDurability(GlobalCoord, NewDurability);
    }

    AddDirtyChunk(GlobalCoord, DirtyChunks);
  };

  ProcessVoxel(Center, Radius, ConstructLogic);
}

void AVoxelWorld::GetVoxelCoordsInRadius(
    const FVector &Center, const float Radius,
    TSet<FIntVector> &FoundGlobalCoords) const {
  const FIntVector CenterGlobalCoord = VoxelData->WorldToGlobalCoord(Center);
  const int32 RadiusInVoxels = FMath::CeilToInt(Radius / VoxelSize);
  const float RadiusSquared = FMath::Square(Radius);

  for (int32 z = CenterGlobalCoord.Z - RadiusInVoxels;
       z <= CenterGlobalCoord.Z + RadiusInVoxels; ++z) {
    for (int32 y = CenterGlobalCoord.Y - RadiusInVoxels;
         y <= CenterGlobalCoord.Y + RadiusInVoxels; ++y) {
      for (int32 x = CenterGlobalCoord.X - RadiusInVoxels;
           x <= CenterGlobalCoord.X + RadiusInVoxels; ++x) {
        const FIntVector GlobalCoord(x, y, z);

        if (FVector::DistSquared(VoxelData->GlobalToWorldCoord(GlobalCoord),
                                 Center) >= RadiusSquared) {
          continue;
        }

        FoundGlobalCoords.Add(GlobalCoord);
      }
    }
  }
}

void AVoxelWorld::ProcessVoxel(
    const FVector &Center, const float Radius,
    const TFunction<void(const FIntVector &, TSet<FIntVector> &)>
        &VoxelModifier) const {
  TSet<FIntVector> GlobalCoordsInRadius;
  TSet<FIntVector> GlobalCoordsToProcess;
  GetVoxelCoordsInRadius(Center, Radius, GlobalCoordsInRadius);

  TSet<FIntVector> DirtyChunkCoords;

  for (const FIntVector &VoxelCoord : GlobalCoordsInRadius) {
    if (VoxelData->GetVoxelID(VoxelCoord) == UVoxelData::GetVoidID()) {
      continue;
    }

    if (IsSurfaceVoxel(VoxelCoord)) {
      GlobalCoordsToProcess.Add(VoxelCoord);
    }
  }

  for (const FIntVector &VoxelCoord : GlobalCoordsToProcess) {
    VoxelModifier(VoxelCoord, DirtyChunkCoords);
  }

  UpdateDirtyChunk(DirtyChunkCoords);
}

void AVoxelWorld::AddDirtyChunk(const FIntVector &GlobalCoord,
                                TSet<FIntVector> &DirtyChunkCoords) const {
  const FIntVector ChunkCoord = VoxelData->GlobalToChunkCoord(GlobalCoord);

  const int32 LocalX = GlobalCoord.X % ChunkSize;
  const int32 LocalY = GlobalCoord.Y % ChunkSize;
  const int32 LocalZ = GlobalCoord.Z % ChunkSize;

  const int32 LastVoxelIndexInChunk = ChunkSize - 1;

  const int32 StartX = (LocalX == 0) ? -1 : 0;
  const int32 EndX = (LocalX == LastVoxelIndexInChunk) ? 1 : 0;

  const int32 StartY = (LocalY == 0) ? -1 : 0;
  const int32 EndY = (LocalY == LastVoxelIndexInChunk) ? 1 : 0;

  const int32 StartZ = (LocalZ == 0) ? -1 : 0;
  const int32 EndZ = (LocalZ == LastVoxelIndexInChunk) ? 1 : 0;

  for (int32 i = StartX; i <= EndX; ++i) {
    for (int32 j = StartY; j <= EndY; ++j) {
      for (int32 k = StartZ; k <= EndZ; ++k) {
        DirtyChunkCoords.Add(ChunkCoord + FIntVector(i, j, k));
      }
    }
  }
}

void AVoxelWorld::UpdateDirtyChunk(
    const TSet<FIntVector> &DirtyChunkCoords) const {
  for (const FIntVector &CoordToUpdate : DirtyChunkCoords) {
    if (CoordToUpdate.X >= 0 && CoordToUpdate.X < WorldSizeInChunks.X &&
        CoordToUpdate.Y >= 0 && CoordToUpdate.Y < WorldSizeInChunks.Y &&
        CoordToUpdate.Z >= 0 && CoordToUpdate.Z < WorldSizeInChunks.Z) {
      if (const FChunk *Chunk = VoxelData->GetChunk(CoordToUpdate)) {
        if (Chunk->VoxelChunk) {
          Chunk->VoxelChunk->UpdateMesh();
        }
      }
    }
  }
}

bool AVoxelWorld::IsSurfaceVoxel(const FIntVector &VoxelCoord) const {
  const TArray NeighborOffsets = {FIntVector(1, 0, 0), FIntVector(-1, 0, 0),
                                  FIntVector(0, 1, 0), FIntVector(0, -1, 0),
                                  FIntVector(0, 0, 1), FIntVector(0, 0, -1)};

  for (const FIntVector &Offset : NeighborOffsets) {
    if (VoxelData->GetVoxelID(VoxelCoord + Offset) == UVoxelData::GetVoidID()) {
      return true;
    }
  }

  return false;
}
