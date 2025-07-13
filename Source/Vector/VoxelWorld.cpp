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
                        VoxelBorderDataAsset, VoxelDebug);

  float DefaultBlockMaxDurability;
  const UVoxelBlockDataAsset *DefaultBlockData = Cast<UVoxelBlockDataAsset>(
      VoxelData->GetVoxelDataAsset(VoxelData->GetDefaultBlockID()));
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
          FIntVector GlobalBlockCoord(x, y, z);
          FVector WorldPosOfBlock = FVector(GlobalBlockCoord) * VoxelSize;

          if (FVector::DistSquared(WorldPosOfBlock, NewRoom.Center) <
              FMath::Square(NewRoom.Radius)) {
            VoxelData->SetVoxel(GlobalBlockCoord,
                                FVoxel(VoxelData->GetVoidID(), 0.f), false);
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
    if (!Cast<UVoxelBlockDataAsset>(
            VoxelData->GetVoxelDataAsset(VoxelData->GetVoxelID(VoxelCoord)))) {
      return;
    }

    const float NewDurability =
        VoxelData->GetDurability(VoxelCoord) - DamageAmount;

    if (NewDurability <= 0) {
      VoxelData->SetVoxel(VoxelCoord, FVoxel(VoxelData->GetVoidID(), 0.f));
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
  auto ConstructLogic = [&](const FIntVector &VoxelCoord,
                            TSet<FIntVector> &DirtyChunks) {
    const UVoxelBlockDataAsset *VoxelDataAsset = Cast<UVoxelBlockDataAsset>(
        VoxelData->GetVoxelDataAsset(VoxelData->GetVoxelID(VoxelCoord)));
    if (!VoxelData) {
      return;
    }

    const UVoxelBlockDataAsset *NewVoxelData = Cast<UVoxelBlockDataAsset>(
        VoxelData->GetVoxelDataAsset(VoxelIDToConstruct));
    if (!NewVoxelData) {
      return;
    }

    const float NewDurability =
        VoxelData->GetDurability(VoxelCoord) + ConstructionAmount;

    if (NewDurability >= VoxelDataAsset->MaxDurability) {
      VoxelData->SetDurability(VoxelCoord, VoxelDataAsset->MaxDurability);

      const TArray<FIntVector> NeighborOffsets = {
          FIntVector(1, 0, 0),  FIntVector(-1, 0, 0), FIntVector(0, 1, 0),
          FIntVector(0, -1, 0), FIntVector(0, 0, 1),  FIntVector(0, 0, -1)};

      for (const FIntVector &NeighborOffset : NeighborOffsets) {
        const FIntVector NeighborCoord = VoxelCoord + NeighborOffset;
        if (VoxelData->GetVoxelID(NeighborCoord) == VoxelData->GetVoidID()) {
          VoxelData->SetVoxel(NeighborCoord, FVoxel(VoxelIDToConstruct, 0.f));

          for (const FIntVector &CheckOffset : NeighborOffsets) {
            const FIntVector CheckCoord = NeighborCoord + CheckOffset;

            if (CheckCoord == VoxelCoord) {
              continue;
            }

            if (const UVoxelBlockDataAsset *EnclosedBlockData =
                    Cast<UVoxelBlockDataAsset>(VoxelData->GetVoxelDataAsset(
                        VoxelData->GetVoxelID(CheckCoord)))) {
              if (!IsSurfaceVoxel(CheckCoord)) {
                VoxelData->SetDurability(CheckCoord,
                                         EnclosedBlockData->MaxDurability);
                AddDirtyChunk(CheckCoord, DirtyChunks);
              }
            }
          }

          if (!IsSurfaceVoxel(NeighborCoord)) {
            VoxelData->SetDurability(NeighborCoord,
                                     NewVoxelData->MaxDurability);
          }

          AddDirtyChunk(NeighborCoord, DirtyChunks);
        }
      }
    } else {
      VoxelData->SetDurability(VoxelCoord, NewDurability);
    }

    AddDirtyChunk(VoxelCoord, DirtyChunks);
  };

  ProcessVoxel(Center, Radius, ConstructLogic);
}

FIntVector AVoxelWorld::WorldPosToVoxelCoord(const FVector &WorldPos) const {
  return FIntVector(FMath::RoundToInt(WorldPos.X / VoxelSize),
                    FMath::RoundToInt(WorldPos.Y / VoxelSize),
                    FMath::RoundToInt(WorldPos.Z / VoxelSize));
}

FIntVector
AVoxelWorld::VoxelCoordToChunkCoord(const FIntVector &VoxelCoord) const {
  return FIntVector(
      FMath::FloorToInt(static_cast<float>(VoxelCoord.X) / ChunkSize),
      FMath::FloorToInt(static_cast<float>(VoxelCoord.Y) / ChunkSize),
      FMath::FloorToInt(static_cast<float>(VoxelCoord.Z) / ChunkSize));
}

void AVoxelWorld::GetVoxelCoordsInRadius(
    const FVector &Center, const float Radius,
    TSet<FIntVector> &FoundVoxelCoords) const {
  const FIntVector CenterVoxelCoord = WorldPosToVoxelCoord(Center);
  const int32 RadiusInVoxels = FMath::CeilToInt(Radius / VoxelSize);
  const float RadiusSquared = FMath::Square(Radius);

  for (int32 z = CenterVoxelCoord.Z - RadiusInVoxels;
       z <= CenterVoxelCoord.Z + RadiusInVoxels; ++z) {
    for (int32 y = CenterVoxelCoord.Y - RadiusInVoxels;
         y <= CenterVoxelCoord.Y + RadiusInVoxels; ++y) {
      for (int32 x = CenterVoxelCoord.X - RadiusInVoxels;
           x <= CenterVoxelCoord.X + RadiusInVoxels; ++x) {
        const FIntVector VoxelCoord(x, y, z);

        const FVector VoxelWorldPos =
            FVector(VoxelCoord) * VoxelSize + (FVector(VoxelSize) * 0.5f);
        if (FVector::DistSquared(VoxelWorldPos, Center) >= RadiusSquared) {
          continue;
        }

        FoundVoxelCoords.Add(VoxelCoord);
      }
    }
  }
}

void AVoxelWorld::ProcessVoxel(
    const FVector &Center, const float Radius,
    const TFunction<void(const FIntVector &, TSet<FIntVector> &)>
        &VoxelModifier) const {
  TSet<FIntVector> VoxelCoordsInRadius;
  TSet<FIntVector> VoxelCoordsToProcess;
  GetVoxelCoordsInRadius(Center, Radius, VoxelCoordsInRadius);

  TSet<FIntVector> DirtyChunkCoords;

  for (const FIntVector &VoxelCoord : VoxelCoordsInRadius) {
    if (VoxelData->GetVoxelID(VoxelCoord) == VoxelData->GetVoidID()) {
      continue;
    }

    if (IsSurfaceVoxel(VoxelCoord)) {
      VoxelCoordsToProcess.Add(VoxelCoord);
    }
  }

  for (const FIntVector &VoxelCoord : VoxelCoordsToProcess) {
    VoxelModifier(VoxelCoord, DirtyChunkCoords);
  }

  UpdateDirtyChunk(DirtyChunkCoords);
}

void AVoxelWorld::AddDirtyChunk(const FIntVector &VoxelCoord,
                                TSet<FIntVector> &DirtyChunks) const {
  const FIntVector ChunkCoord = VoxelCoordToChunkCoord(VoxelCoord);

  const int32 LocalX = VoxelCoord.X % ChunkSize;
  const int32 LocalY = VoxelCoord.Y % ChunkSize;
  const int32 LocalZ = VoxelCoord.Z % ChunkSize;

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
        DirtyChunks.Add(ChunkCoord + FIntVector(i, j, k));
      }
    }
  }
}

void AVoxelWorld::UpdateDirtyChunk(const TSet<FIntVector> &DirtyChunks) const {
  for (const FIntVector &CoordToUpdate : DirtyChunks) {
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
    if (VoxelData->GetVoxelID(VoxelCoord + Offset) == VoxelData->GetVoidID()) {
      return true;
    }
  }

  return false;
}
