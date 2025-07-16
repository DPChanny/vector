#include "Managers/BuildManager.h"

#include "Actors/VoxelWorld.h"
#include "DataAssets/VoxelBlockDataAsset.h"
#include "DataAssets/VoxelBorderDataAsset.h"
#include "DataAssets/VoxelVoidDataAsset.h"
#include "Managers/DataManager.h"
#include "Managers/MeshManager.h"

void UBuildManager::Initialize() {
  if (const TObjectPtr<AVoxelWorld> VoxelWorld =
          Cast<AVoxelWorld>(GetOuter())) {
    DataManager = VoxelWorld->GetDataManager();
    MeshManager = VoxelWorld->GetMeshManager();
  }
}

void UBuildManager::DamageVoxelsInRadius(const FIntVector &CenterGlobalCoord,
                                         const float Radius,
                                         const float DamageAmount) const {
  auto DamageLogic = [&](const FIntVector &VoxelCoord) {
    if (!DataManager) {
      return;
    }

    if (!FVoxelBlockData::IsBlock(DataManager->GetVoxelData(VoxelCoord))) {
      return;
    }

    DataManager->ModifyVoxelData(
        VoxelCoord, [&](FVoxelBaseData *VoxelBaseData) {
          FVoxelBlockData *VoxelBlockData =
              dynamic_cast<FVoxelBlockData *>(VoxelBaseData);

          VoxelBlockData->Durability -= DamageAmount;

          if (VoxelBlockData->Durability <= 0) {
            DataManager->SetVoxelData(VoxelCoord, new FVoxelVoidData());
          }
        });
  };

  ProcessVoxelsInRadius(CenterGlobalCoord, Radius, DamageLogic);
}

void UBuildManager::ConstructVoxelsInRadius(
    const FIntVector &CenterGlobalCoord, const float Radius,
    const float ConstructionAmount,
    const TObjectPtr<UVoxelBlockDataAsset> &NewVoxelBlockDataAsset) const {
  auto ConstructLogic = [&](const FIntVector &GlobalCoord) {
    if (!DataManager) {
      return;
    }

    DataManager->ModifyVoxelData(GlobalCoord, [&](FVoxelBaseData
                                                      *VoxelBaseData) {
      FVoxelSubstanceData *VoxelSubstanceData =
          dynamic_cast<FVoxelSubstanceData *>(VoxelBaseData);
      if (!VoxelSubstanceData) {
        return;
      }

      FVoxelBlockData *VoxelBlockData =
          dynamic_cast<FVoxelBlockData *>(VoxelSubstanceData);

      if (VoxelBlockData) {
        VoxelBlockData->Durability += ConstructionAmount;
      }

      if ((VoxelBlockData &&
           VoxelBlockData->Durability >
               VoxelBlockData->GetBlockDataAsset()->MaxDurability) ||
          FVoxelBorderData::IsBorder(VoxelSubstanceData)) {

        if (VoxelBlockData) {
          VoxelBlockData->Durability =
              VoxelBlockData->GetBlockDataAsset()->MaxDurability;
        }

        for (const FIntVector &NeighborOffset : NeighborOffsets) {
          if (const FIntVector NeighborGlobalCoord =
                  GlobalCoord + NeighborOffset;
              FVoxelVoidData::IsVoid(
                  DataManager->GetVoxelData(NeighborGlobalCoord))) {

            DataManager->SetVoxelData(
                NeighborGlobalCoord, new FVoxelBlockData(NewVoxelBlockDataAsset,
                                                         ConstructionAmount));

            for (const FIntVector &CheckOffset : NeighborOffsets) {
              const FIntVector CheckGlobalCoord =
                  NeighborGlobalCoord + CheckOffset;
              if (CheckGlobalCoord == GlobalCoord) {
                continue;
              }

              if (!IsSurfaceVoxel(CheckGlobalCoord)) {
                DataManager->ModifyVoxelData(
                    CheckGlobalCoord, [&](FVoxelBaseData *CheckBaseData) {
                      if (FVoxelBlockData *CheckBlockData =
                              dynamic_cast<FVoxelBlockData *>(CheckBaseData)) {
                        CheckBlockData->Durability =
                            CheckBlockData->GetBlockDataAsset()->MaxDurability;
                      }
                    });
              }
            }

            if (!IsSurfaceVoxel(NeighborGlobalCoord)) {
              DataManager->ModifyVoxelData(
                  NeighborGlobalCoord, [&](FVoxelBaseData *NeighborBaseData) {
                    if (FVoxelBlockData *NeighborBlockData =
                            dynamic_cast<FVoxelBlockData *>(NeighborBaseData)) {
                      NeighborBlockData->Durability =
                          NeighborBlockData->GetBlockDataAsset()->MaxDurability;
                    }
                  });
            }
          }
        }
      }
    });
  };

  ProcessVoxelsInRadius(CenterGlobalCoord, Radius, ConstructLogic);
}

void UBuildManager::GetGlobalCoordsInRadius(
    const FIntVector &CenterGlobalCoord, const float Radius,
    TSet<FIntVector> &FoundGlobalCoords) const {
  if (!DataManager) {
    return;
  }

  const int32 RadiusInVoxels =
      FMath::CeilToInt(Radius / DataManager->GetVoxelSize());
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

void UBuildManager::ProcessVoxelsInRadius(
    const FIntVector &CenterGlobalCoord, const float Radius,
    const TFunction<void(const FIntVector &)> &VoxelModifier) const {
  if (!MeshManager || !DataManager) {
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

      if (!FVoxelVoidData::IsVoid(DataManager->GetVoxelData(NeighborCoord))) {
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

  MeshManager->FlushDirtyChunks();
}

bool UBuildManager::IsSurfaceVoxel(const FIntVector &VoxelCoord) const {
  if (!DataManager) {
    return false;
  }

  for (const FIntVector &Offset : NeighborOffsets) {
    if (FVoxelVoidData::IsVoid(
            DataManager->GetVoxelData(VoxelCoord + Offset))) {
      return true;
    }
  }
  return false;
}
