#include "EntityChunk.h"

void UEntityChunk::Tick(float DeltaTime) {}

void UEntityChunk::AddEntity(const FIntVector &VoxelCoord) {
  ManagedVoxels.Add(VoxelCoord);
}

void UEntityChunk::RemoveEntity(const FIntVector &VoxelCoord) {
  ManagedVoxels.Remove(VoxelCoord);
}

bool UEntityChunk::IsEmpty() const { return ManagedVoxels.IsEmpty(); }

void UEntityChunk::OnVoxelDataModified(const FIntVector &VoxelCoord) {}
