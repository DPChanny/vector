﻿#include "Managers/DebugManager.h"

#include "Actors/VoxelDebugActor.h"
#include "Actors/VoxelWorldActor.h"
#include "Managers/DataManager.h"

void UDebugManager::Initialize(
    const TSubclassOf<AVoxelDebugActor> &InDebugActorClass) {
  DebugActorClass = InDebugActorClass;
  if (const TObjectPtr<AVoxelWorldActor> VoxelWorld =
          Cast<AVoxelWorldActor>(GetOuter())) {
    DataManager = VoxelWorld->GetDataManager();
  }
}

void UDebugManager::SetDebugVoxel(const FIntVector &NewDebugVoxel,
                                  const FColor &Color) {
  DebugVoxelsBuffer.Emplace(NewDebugVoxel, Color);
}

void UDebugManager::SetDebugVoxels(const TSet<FIntVector> &NewDebugVoxels,
                                   const FColor &Color) {
  for (const FIntVector &NewDebugVoxel : NewDebugVoxels) {
    SetDebugVoxel(NewDebugVoxel, Color);
  }
}

void UDebugManager::FlushDebugVoxelBuffer() {
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

void UDebugManager::AddDebugVoxel(const FIntVector &GlobalCoord) {
  const TObjectPtr<UWorld> World = GetWorld();

  if (DebugVoxels.Contains(GlobalCoord) || !DataManager || !DebugActorClass ||
      !World) {
    return;
  }

  FActorSpawnParameters SpawnParams;
  SpawnParams.Owner = Cast<AActor>(GetOuter());
  const TObjectPtr<AVoxelDebugActor> NewDebugActor =
      World->SpawnActor<AVoxelDebugActor>(
          DebugActorClass, DataManager->GlobalToWorldCoord(GlobalCoord),
          FRotator::ZeroRotator, SpawnParams);

  if (NewDebugActor) {
    NewDebugActor->Initialize(GlobalCoord, DebugVoxelsBuffer[GlobalCoord]);
    DebugVoxels.Add(GlobalCoord, NewDebugActor);
  }
}

void UDebugManager::RemoveDebugVoxel(const FIntVector &VoxelCoord) {
  const TObjectPtr<AVoxelDebugActor> FoundActor =
      DebugVoxels.FindRef(VoxelCoord);
  if (FoundActor) {
    FoundActor->Destroy();
  }
  DebugVoxels.Remove(VoxelCoord);
}
