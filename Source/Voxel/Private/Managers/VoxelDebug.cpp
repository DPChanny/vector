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

void UVoxelDebug::SetDebugVoxel(const FIntVector &NewDebugVoxel,
                                const FColor &Color) {
  DebugVoxelsBuffer.Emplace(NewDebugVoxel, Color);
}

void UVoxelDebug::SetDebugVoxels(const TSet<FIntVector> &NewDebugVoxels,
                                 const FColor &Color) {
  for (const FIntVector &NewDebugVoxel : NewDebugVoxels) {
    SetDebugVoxel(NewDebugVoxel, Color);
  }
}

void UVoxelDebug::FlushDebugVoxelBuffer() {
  TSet<FIntVector> CurrentDebugVoxels;
  DebugVoxels.GetKeys(CurrentDebugVoxels);

  TSet<FIntVector> BufferVoxels;
  DebugVoxelsBuffer.GetKeys(BufferVoxels);

  for (const FIntVector &VoxelToAdd :
       BufferVoxels.Difference(CurrentDebugVoxels)) {
    AddDebugVoxel(VoxelToAdd);
  }

  for (const FIntVector &VoxelToRemove :
       CurrentDebugVoxels.Difference(BufferVoxels)) {
    RemoveDebugVoxel(VoxelToRemove);
  }

  for (const FIntVector &VoxelToUpdate :
       CurrentDebugVoxels.Intersect(BufferVoxels)) {
    if (const TObjectPtr<AVoxelDebugActor> FoundActor =
            DebugVoxels.FindRef(VoxelToUpdate)) {
      FoundActor->UpdateActor(DebugVoxelsBuffer[VoxelToUpdate]);
    }
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
    NewDebugActor->Initialize(GlobalCoord, DebugVoxelsBuffer[GlobalCoord]);
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
