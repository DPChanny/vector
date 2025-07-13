#include "VoxelDebug.h"
#include "Engine/World.h"
#include "VoxelData.h"
#include "VoxelDebugActor.h"
#include "VoxelWorld.h"

void UVoxelDebug::Initialize(
    const TSubclassOf<AVoxelDebugActor> &InDebugActorClass) {
  DebugActorClass = InDebugActorClass;
  VoxelData = Cast<AVoxelWorld>(GetOuter())->GetVoxelData();
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
    if (const TObjectPtr<AVoxelDebugActor> *FoundActor =
            DebugVoxels.Find(VoxelToUpdate)) {
      (*FoundActor)->UpdateWidget();
    }
  }

  const TSet<FIntVector> VoxelsToAdd =
      DebugVoxelsBuffer.Difference(CurrentDebugVoxels);
  for (const FIntVector &VoxelToAdd : VoxelsToAdd) {
    AddDebugVoxel(VoxelToAdd);
  }

  DebugVoxelsBuffer.Reset();
}

void UVoxelDebug::AddDebugVoxel(const FIntVector &VoxelCoord) {
  if (DebugVoxels.Contains(VoxelCoord)) {
    return;
  }

  if (!DebugActorClass) {
    UE_LOG(LogTemp, Warning, TEXT("DebugActorClass is not set in VoxelDebug."));
    return;
  }

  const FVector SpawnLocation =
      (FVector(VoxelCoord) * VoxelData->GetVoxelSize());

  UWorld *World = GetWorld();
  if (!World) {
    return;
  }

  AVoxelDebugActor *NewDebugActor = World->SpawnActor<AVoxelDebugActor>(
      DebugActorClass, SpawnLocation, FRotator::ZeroRotator);

  if (NewDebugActor && VoxelData) {
    NewDebugActor->Initialize(VoxelCoord, VoxelData);
    DebugVoxels.Add(VoxelCoord, NewDebugActor);
  }
}

void UVoxelDebug::RemoveDebugVoxel(const FIntVector &VoxelCoord) {
  if (const TObjectPtr<AVoxelDebugActor> FoundActor =
          DebugVoxels.FindRef(VoxelCoord)) {
    if (FoundActor) {
      FoundActor->Destroy();
    }
    DebugVoxels.Remove(VoxelCoord);
  }
}
