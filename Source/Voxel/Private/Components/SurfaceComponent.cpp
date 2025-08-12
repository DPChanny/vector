#include "Components/SurfaceComponent.h"
#include "Actors/EntityChunkActor.h"
#include "Managers/DataManager.h"

const TArray<FIntVector> USurfaceComponent::NeighborOffsets = {
    FIntVector(1, 0, 0),  FIntVector(-1, 0, 0), FIntVector(0, 1, 0),
    FIntVector(0, -1, 0), FIntVector(0, 0, 1),  FIntVector(0, 0, -1)};

USurfaceComponent::USurfaceComponent() {
  PrimaryComponentTick.bCanEverTick = false;
}

FIntVector USurfaceComponent::GetClosestSurfaceVoxel(
    const FVector& TargetCoord) const {
  if (SurfaceVoxels.IsEmpty()) {
    return FIntVector::ZeroValue;
  }

  const FIntVector TargetGlobalCoord =
      EntityChunkActor->GetDataManager()->WorldToGlobalCoord(TargetCoord);

  FIntVector ClosestGlobalCoord = *SurfaceVoxels.begin();
  float MinDistanceSquared = FVector::DistSquared(FVector(TargetGlobalCoord),
                                                  FVector(ClosestGlobalCoord));

  for (const FIntVector& GlobalCoord : SurfaceVoxels) {
    const float DistanceSquared =
        FVector::DistSquared(FVector(TargetGlobalCoord), FVector(GlobalCoord));

    if (DistanceSquared < MinDistanceSquared) {
      MinDistanceSquared = DistanceSquared;
      ClosestGlobalCoord = GlobalCoord;
    }
  }

  return ClosestGlobalCoord;
}

void USurfaceComponent::OnEntityAdded(const FIntVector& GlobalCoord,
                                      const FVoxelEntityData& NewEntityData) {
  UpdateSurfaceAroundVoxel(GlobalCoord);
}

void USurfaceComponent::OnEntityRemoved(const FIntVector& GlobalCoord) {
  SurfaceVoxels.Remove(GlobalCoord);
  UpdateSurfaceAroundVoxel(GlobalCoord);
}

void USurfaceComponent::OnEntityModified(
    const FIntVector& GlobalCoord, const FVoxelEntityData& NewEntityData) {
  UpdateSurfaceAroundVoxel(GlobalCoord);
}

bool USurfaceComponent::IsSurfaceVoxel(const FIntVector& GlobalCoord) const {
  if (!EntityChunkActor) {
    return false;
  }

  const TSet<FIntVector>& EntityCoords = EntityChunkActor->GetEntityCoords();

  if (!EntityCoords.Contains(GlobalCoord)) {
    return false;
  }

  for (const FIntVector& Offset : NeighborOffsets) {
    if (!EntityCoords.Contains(GlobalCoord + Offset)) {
      return true;
    }
  }

  return false;
}

void USurfaceComponent::UpdateSurfaceAroundVoxel(
    const FIntVector& GlobalCoord) {
  if (!EntityChunkActor) {
    return;
  }

  if (IsSurfaceVoxel(GlobalCoord)) {
    SurfaceVoxels.Add(GlobalCoord);
  } else {
    SurfaceVoxels.Remove(GlobalCoord);
  }

  for (const FIntVector& Offset : NeighborOffsets) {
    if (const FIntVector NeighborCoord = GlobalCoord + Offset;
        IsSurfaceVoxel(NeighborCoord)) {
      SurfaceVoxels.Add(NeighborCoord);
    } else {
      SurfaceVoxels.Remove(NeighborCoord);
    }
  }
}
