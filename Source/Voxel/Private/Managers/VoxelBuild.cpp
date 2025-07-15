#include "Managers/VoxelBuild.h"
#include "Actors/VoxelWorld.h"
#include "DataAssets/VoxelBlockDataAsset.h"
#include "Managers/VoxelData.h"
#include "Managers/VoxelMesh.h"

void UVoxelBuild::Initialize() {
  if (const TObjectPtr<AVoxelWorld> VoxelWorld =
          Cast<AVoxelWorld>(GetOuter())) {
    VoxelData = VoxelWorld->GetVoxelData();
    VoxelMesh = VoxelWorld->GetVoxelMesh();
  }
}

void UVoxelBuild::DamageVoxelsInRadius(const FIntVector &CenterGlobalCoord,
                                       const float Radius,
                                       const float DamageAmount) const {
  auto DamageLogic = [&](const FIntVector &VoxelCoord) {
    if (!VoxelData) {
      return;
    }

    if (!VoxelData->GetVoxelDataAsset<UVoxelBlockDataAsset>(VoxelCoord)) {
      return;
    }

    const float NewDurability =
        VoxelData->GetDurability(VoxelCoord) - DamageAmount;

    if (NewDurability <= 0) {
      VoxelData->SetVoxel(VoxelCoord, FVoxel(UVoxelData::GetVoidID(), 0.f));
    } else {
      VoxelData->SetDurability(VoxelCoord, NewDurability);
    }
  };

  ProcessVoxelsInRadius(CenterGlobalCoord, Radius, DamageLogic);
}

void UVoxelBuild::ConstructVoxelsInRadius(
    const FIntVector &CenterGlobalCoord, const float Radius,
    const float ConstructionAmount, const int32 VoxelIDToConstruct) const {
  auto ConstructLogic = [&](const FIntVector &GlobalCoord) {
    if (!VoxelData) {
      return;
    }

    const TObjectPtr<UVoxelBlockDataAsset> NewVoxelData =
        VoxelData->GetVoxelDataAsset<UVoxelBlockDataAsset>(VoxelIDToConstruct);
    if (!NewVoxelData) {
      return;
    }

    const float NewDurability =
        VoxelData->GetDurability(GlobalCoord) + ConstructionAmount;

    const TObjectPtr<UVoxelBlockDataAsset> BlockDataAsset =
        VoxelData->GetVoxelDataAsset<UVoxelBlockDataAsset>(GlobalCoord);

    if ((BlockDataAsset && NewDurability > BlockDataAsset->MaxDurability) ||
        VoxelData->GetVoxelID(GlobalCoord) == UVoxelData::GetBorderID()) {
      if (BlockDataAsset) {
        VoxelData->SetDurability(GlobalCoord, BlockDataAsset->MaxDurability);
      }

      for (const FIntVector &NeighborOffset : NeighborOffsets) {
        if (const FIntVector NeighborGlobalCoord = GlobalCoord + NeighborOffset;
            VoxelData->GetVoxelID(NeighborGlobalCoord) ==
            UVoxelData::GetVoidID()) {
          VoxelData->SetVoxel(NeighborGlobalCoord,
                              FVoxel(VoxelIDToConstruct, ConstructionAmount));

          for (const FIntVector &CheckOffset : NeighborOffsets) {
            const FIntVector CheckGlobalCoord =
                NeighborGlobalCoord + CheckOffset;
            if (CheckGlobalCoord == GlobalCoord) {
              continue;
            }

            if (const TObjectPtr<UVoxelBlockDataAsset> CheckBlockDataAsset =
                    VoxelData->GetVoxelDataAsset<UVoxelBlockDataAsset>(
                        CheckGlobalCoord)) {
              if (!IsSurfaceVoxel(CheckGlobalCoord)) {
                VoxelData->SetDurability(CheckGlobalCoord,
                                         CheckBlockDataAsset->MaxDurability);
              }
            }
          }

          if (!IsSurfaceVoxel(NeighborGlobalCoord)) {
            VoxelData->SetDurability(NeighborGlobalCoord,
                                     NewVoxelData->MaxDurability);
          }
        }
      }
    } else if (BlockDataAsset) {
      VoxelData->SetDurability(GlobalCoord, NewDurability);
    }
  };

  ProcessVoxelsInRadius(CenterGlobalCoord, Radius, ConstructLogic);
}

void UVoxelBuild::GetGlobalCoordsInRadius(
    const FIntVector &CenterGlobalCoord, const float Radius,
    TSet<FIntVector> &FoundGlobalCoords) const {
  if (!VoxelData) {
    return;
  }

  const int32 RadiusInVoxels =
      FMath::CeilToInt(Radius / VoxelData->GetVoxelSize());
  const int32 RadiusSquared = FMath::Square(RadiusInVoxels);

  for (int32 z = -RadiusInVoxels; z <= RadiusInVoxels; ++z) {
    for (int32 y = -RadiusInVoxels; y <= RadiusInVoxels; ++y) {
      for (int32 x = -RadiusInVoxels; x <= RadiusInVoxels; ++x) {
        if (const int32 DistanceSquared = x * x + y * y + z * z;
            DistanceSquared <= RadiusSquared) {
          const FIntVector GlobalCoord =
              CenterGlobalCoord + FIntVector(x, y, z);
          FoundGlobalCoords.Add(GlobalCoord);
        }
      }
    }
  }
}

void UVoxelBuild::ProcessVoxelsInRadius(
    const FIntVector &CenterGlobalCoord, const float Radius,
    const TFunction<void(const FIntVector &)> &VoxelModifier) const {
  if (!VoxelMesh || !VoxelData) {
    return;
  }

  TSet<FIntVector> GlobalCoordsInRadius;
  GetGlobalCoordsInRadius(CenterGlobalCoord, Radius, GlobalCoordsInRadius);

  TSet<FIntVector> GlobalCoordsToProcess;
  TQueue<FIntVector> Queue;
  TSet<FIntVector> Visited;

  Queue.Enqueue(CenterGlobalCoord);
  Visited.Add(CenterGlobalCoord);

  while (!Queue.IsEmpty()) {
    FIntVector CurrentGlobalCoord;
    Queue.Dequeue(CurrentGlobalCoord);

    if (IsSurfaceVoxel(CurrentGlobalCoord)) {
      GlobalCoordsToProcess.Add(CurrentGlobalCoord);
    }

    for (const FIntVector &Offset : NeighborOffsets) {
      const FIntVector NeighborCoord = CurrentGlobalCoord + Offset;

      if (Visited.Contains(NeighborCoord) ||
          !GlobalCoordsInRadius.Contains(NeighborCoord)) {
        continue;
      }

      if (VoxelData->GetVoxelID(NeighborCoord) != UVoxelData::GetVoidID()) {
        Queue.Enqueue(NeighborCoord);
        Visited.Add(NeighborCoord);
      } else {
        Visited.Add(NeighborCoord);
      }
    }
  }

  for (const FIntVector &VoxelCoord : GlobalCoordsToProcess) {
    VoxelModifier(VoxelCoord);
  }

  VoxelMesh->FlushDirtyChunks();
}

bool UVoxelBuild::IsSurfaceVoxel(const FIntVector &VoxelCoord) const {
  if (!VoxelData) {
    return false;
  }

  for (const FIntVector &Offset : NeighborOffsets) {
    if (VoxelData->GetVoxelID(VoxelCoord + Offset) == UVoxelData::GetVoidID()) {
      return true;
    }
  }
  return false;
}
