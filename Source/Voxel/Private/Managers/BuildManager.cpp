#include "Managers/BuildManager.h"

#include "DataAssets/VoxelBlockDataAsset.h"
#include "DataAssets/VoxelBorderDataAsset.h"
#include "DataAssets/VoxelVoidDataAsset.h"
#include "Managers/DataManager.h"
#include "Managers/EntityManager.h"
#include "Managers/MeshManager.h"

void UBuildManager::InitializeComponent() {
  Super::InitializeComponent();

  if (const AActor* Owner = GetOwner()) {
    DataManager = Owner->GetComponentByClass<UDataManager>();
    MeshManager = Owner->GetComponentByClass<UMeshManager>();
    EntityManager = Owner->GetComponentByClass<UEntityManager>();
  }
}

UBuildManager::UBuildManager() {
  bWantsInitializeComponent = true;
}

void UBuildManager::DamageBlocksInRadius(const FIntVector& CenterGlobalCoord,
                                         const float Radius,
                                         const float DamageAmount) const {
  auto DamageLogic = [&](const FIntVector& GlobalCoord) {
    if (!DataManager) {
      return;
    }

    const FVoxelBlockData* BlockData = dynamic_cast<const FVoxelBlockData*>(
        DataManager->GetVoxelData(GlobalCoord));
    if (!BlockData) {
      return;
    }

    DataManager->ModifyVoxelData(
        GlobalCoord, [DamageAmount](FVoxelBaseData* ModifierBaseData) {
          dynamic_cast<FVoxelBlockData*>(ModifierBaseData)->Durability -=
              DamageAmount;
        });

    if (BlockData->Durability <= 0) {
      DataManager->SetVoxelData(GlobalCoord, new FVoxelVoidData());
    }
  };

  ProcessVoxelsInRadius(CenterGlobalCoord, Radius, DamageLogic);
}

void UBuildManager::ConstructBlocksInRadius(
    const FIntVector& CenterGlobalCoord, const float Radius,
    const float ConstructionAmount,
    const TObjectPtr<const UVoxelBlockDataAsset>& NewVoxelBlockDataAsset,
    const FVoxelBlockParams& VoxelParams) const {
  auto ConstructLogic = [&](const FIntVector& GlobalCoord) {
    if (!DataManager) {
      return;
    }

    const FVoxelBaseData* VoxelData = DataManager->GetVoxelData(GlobalCoord);

    const FVoxelBlockData* BlockData =
        dynamic_cast<const FVoxelBlockData*>(VoxelData);

    if (BlockData) {
      DataManager->ModifyVoxelData(
          GlobalCoord, [ConstructionAmount](FVoxelBaseData* ModifierBaseData) {
            dynamic_cast<FVoxelBlockData*>(ModifierBaseData)->Durability +=
                ConstructionAmount;
          });
    }

    if ((BlockData && BlockData->Durability >
                          BlockData->GetBlockDataAsset()->MaxDurability) ||
        FVoxelBorderData::IsBorder(VoxelData)) {

      if (BlockData) {
        DataManager->ModifyVoxelData(
            GlobalCoord, [&](FVoxelBaseData* ModifierBaseData) {
              FVoxelBlockData* ModifierBlockData =
                  dynamic_cast<FVoxelBlockData*>(ModifierBaseData);
              ModifierBlockData->Durability =
                  ModifierBlockData->GetBlockDataAsset()->MaxDurability;
            });
      }

      for (const FIntVector& NeighborOffset : NeighborOffsets) {
        if (const FIntVector NeighborGlobalCoord = GlobalCoord + NeighborOffset;
            FVoxelVoidData::IsVoid(
                DataManager->GetVoxelData(NeighborGlobalCoord))) {

          const TUniquePtr<FVoxelBlockParams> NewVoxelParams(
              static_cast<FVoxelBlockParams*>(VoxelParams.Clone()));
          NewVoxelParams->Durability = ConstructionAmount;

          if (FVoxelBaseData* NewVoxelData =
                  NewVoxelBlockDataAsset->ConstructVoxelData(*NewVoxelParams)) {
            DataManager->SetVoxelData(NeighborGlobalCoord, NewVoxelData);
          }

          for (const FIntVector& CheckOffset : NeighborOffsets) {
            const FIntVector CheckGlobalCoord =
                NeighborGlobalCoord + CheckOffset;
            if (CheckGlobalCoord == GlobalCoord) {
              continue;
            }

            if (!IsSurfaceVoxel(CheckGlobalCoord)) {
              DataManager->ModifyVoxelData(
                  CheckGlobalCoord, [&](FVoxelBaseData* CheckVoxelData) {
                    if (FVoxelBlockData* CheckBlockData =
                            dynamic_cast<FVoxelBlockData*>(CheckVoxelData)) {
                      CheckBlockData->Durability =
                          CheckBlockData->GetBlockDataAsset()->MaxDurability;
                    }
                  });
            }
          }

          if (!IsSurfaceVoxel(NeighborGlobalCoord)) {
            DataManager->ModifyVoxelData(
                NeighborGlobalCoord, [&](FVoxelBaseData* NeighborVoxelData) {
                  if (FVoxelBlockData* NeighborBlockData =
                          dynamic_cast<FVoxelBlockData*>(NeighborVoxelData)) {
                    NeighborBlockData->Durability =
                        NeighborBlockData->GetBlockDataAsset()->MaxDurability;
                  }
                });
          }
        }
      }
    }
  };

  ProcessVoxelsInRadius(CenterGlobalCoord, Radius, ConstructLogic);
}

void UBuildManager::GetGlobalCoordsInRadius(
    const FIntVector& CenterGlobalCoord, const float Radius,
    TSet<FIntVector>& FoundGlobalCoords) const {
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
    const FIntVector& CenterGlobalCoord, const float Radius,
    const TFunction<void(const FIntVector&)>& VoxelModifier) const {
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

    for (const FIntVector& Offset : NeighborOffsets) {
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

  for (const FIntVector& VoxelCoord : GlobalCoordsToProcess) {
    VoxelModifier(VoxelCoord);
  }

  if (MeshManager) {
    MeshManager->FlushDirtyChunks();
  }

  if (EntityManager) {
    EntityManager->FlushDirtyEntities();
  }
}

bool UBuildManager::IsSurfaceVoxel(const FIntVector& VoxelCoord) const {
  if (!DataManager) {
    return false;
  }

  for (const FIntVector& Offset : NeighborOffsets) {
    if (FVoxelVoidData::IsVoid(
            DataManager->GetVoxelData(VoxelCoord + Offset))) {
      return true;
    }
  }
  return false;
}
