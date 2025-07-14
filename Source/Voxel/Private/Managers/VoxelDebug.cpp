#include "Managers/VoxelDebug.h"

#include "Actors/VoxelDebugActor.h"
#include "Actors/VoxelWorld.h"
#include "Managers/VoxelData.h"

void UVoxelDebug::Initialize(
    const TSubclassOf<AVoxelDebugActor> &InDebugActorClass) {
  DebugActorClass = InDebugActorClass;
  if (const TObjectPtr<AVoxelWorld> VoxelWorld =
          Cast<AVoxelWorld>(GetOuter())) {
    VoxelData = VoxelWorld->GetVoxelData();
  }
}

void UVoxelDebug::SetDebugVoxel(const FIntVector &NewDebugVoxel) {
  DebugVoxelsBuffer.Add(NewDebugVoxel);
}

void UVoxelDebug::SetDebugVoxels(const TSet<FIntVector> &NewDebugVoxels) {
  DebugVoxelsBuffer.Append(NewDebugVoxels);
}

void UVoxelDebug::FlushDebugVoxelBuffer() {
  TSet<FIntVector> CurrentDebugVoxels;
  DebugVoxels.GetKeys(CurrentDebugVoxels);

  const TSet<FIntVector> VoxelsToRemove =
      CurrentDebugVoxels.Difference(DebugVoxelsBuffer);
  for (const FIntVector &VoxelToRemove : VoxelsToRemove) {
    RemoveDebugVoxel(VoxelToRemove);
  }

  const TSet<FIntVector> VoxelsToUpdate =
      CurrentDebugVoxels.Difference(VoxelsToRemove);
  for (const FIntVector &VoxelToUpdate : VoxelsToUpdate) {
    if (const TObjectPtr<AVoxelDebugActor> FoundActor =
            DebugVoxels.FindRef(VoxelToUpdate)) {
      FoundActor->UpdateWidget();
    }
  }

  const TSet<FIntVector> VoxelsToAdd =
      DebugVoxelsBuffer.Difference(CurrentDebugVoxels);
  for (const FIntVector &VoxelToAdd : VoxelsToAdd) {
    AddDebugVoxel(VoxelToAdd);
  }

  DebugVoxelsBuffer.Empty();
}

void UVoxelDebug::AddDebugVoxel(const FIntVector &GlobalCoord) {
  const TObjectPtr<UWorld> World = GetWorld();

  if (DebugVoxels.Contains(GlobalCoord) || !VoxelData || !DebugActorClass ||
      !World) {
    return;
  }

  FActorSpawnParameters SpawnParams;
  SpawnParams.Owner = Cast<AActor>(GetOuter());
  const TObjectPtr<AVoxelDebugActor> NewDebugActor =
      World->SpawnActor<AVoxelDebugActor>(
          DebugActorClass, VoxelData->GlobalToWorldCoord(GlobalCoord),
          FRotator::ZeroRotator, SpawnParams);

  if (NewDebugActor) {
    NewDebugActor->Initialize(GlobalCoord);
    DebugVoxels.Add(GlobalCoord, NewDebugActor);
  }
}

void UVoxelDebug::RemoveDebugVoxel(const FIntVector &VoxelCoord) {
  const TObjectPtr<AVoxelDebugActor> FoundActor =
      DebugVoxels.FindRef(VoxelCoord);
  if (FoundActor) {
    FoundActor->Destroy();
  }
  DebugVoxels.Remove(VoxelCoord);
}
