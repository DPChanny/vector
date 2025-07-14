#include "VoxelBuild.h"
#include "VoxelBlockDataAsset.h"
#include "VoxelData.h"
#include "VoxelMesh.h"
#include "VoxelWorld.h"

void UVoxelBuild::Initialize() {
  if (const TObjectPtr<AVoxelWorld> VoxelWorld =
          Cast<AVoxelWorld>(GetOuter())) {
    VoxelData = VoxelWorld->GetVoxelData();
    VoxelMesh = VoxelWorld->GetVoxelMesh();
  }
}

void UVoxelBuild::DamageVoxel(const FVector &Center, const float Radius,
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

  ProcessVoxel(Center, Radius, DamageLogic);
}

void UVoxelBuild::ConstructVoxel(const FVector &Center, const float Radius,
                                 const float ConstructionAmount,
                                 const int32 VoxelIDToConstruct) const {
  auto ConstructLogic = [&](const FIntVector &GlobalCoord) {
    if (!VoxelData) {
      return;
    }

    const TObjectPtr<UVoxelBlockDataAsset> VoxelDataAsset =
        VoxelData->GetVoxelDataAsset<UVoxelBlockDataAsset>(GlobalCoord);
    if (!VoxelDataAsset) {
      return;
    }

    const TObjectPtr<UVoxelBlockDataAsset> NewVoxelData =
        VoxelData->GetVoxelDataAsset<UVoxelBlockDataAsset>(VoxelIDToConstruct);
    if (!NewVoxelData) {
      return;
    }

    const float NewDurability =
        VoxelData->GetDurability(GlobalCoord) + ConstructionAmount;

    if (NewDurability > VoxelDataAsset->MaxDurability) {
      VoxelData->SetDurability(GlobalCoord, VoxelDataAsset->MaxDurability);

      const TArray NeighborOffsets = {
          FIntVector(1, 0, 0),  FIntVector(-1, 0, 0), FIntVector(0, 1, 0),
          FIntVector(0, -1, 0), FIntVector(0, 0, 1),  FIntVector(0, 0, -1)};

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
    } else {
      VoxelData->SetDurability(GlobalCoord, NewDurability);
    }
  };

  ProcessVoxel(Center, Radius, ConstructLogic);
}

void UVoxelBuild::GetGlobalCoordsInRadius(
    const FVector &Center, const float Radius,
    TSet<FIntVector> &FoundGlobalCoords) const {
  if (!VoxelData) {
    return;
  }
  const FIntVector CenterGlobalCoord = VoxelData->WorldToGlobalCoord(Center);
  const int32 RadiusInVoxels =
      FMath::CeilToInt(Radius / VoxelData->GetVoxelSize());
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

void UVoxelBuild::ProcessVoxel(
    const FVector &Center, const float Radius,
    const TFunction<void(const FIntVector &)> &VoxelModifier) const {
  if (!VoxelMesh) {
    return;
  }

  TSet<FIntVector> GlobalCoordsInRadius;
  TSet<FIntVector> GlobalCoordsToProcess;
  GetGlobalCoordsInRadius(Center, Radius, GlobalCoordsInRadius);

  for (const FIntVector &VoxelCoord : GlobalCoordsInRadius) {
    if (VoxelData->GetVoxelID(VoxelCoord) == UVoxelData::GetVoidID()) {
      continue;
    }
    if (IsSurfaceVoxel(VoxelCoord)) {
      GlobalCoordsToProcess.Add(VoxelCoord);
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
