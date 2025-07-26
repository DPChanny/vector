#include "EntityChunk.h"

void UEntityChunk::Tick(float DeltaTime) {}

void UEntityChunk::AddVoxel(const FIntVector &VoxelCoord) {
  ManagedVoxels.Add(VoxelCoord);
}

void UEntityChunk::RemoveVoxel(const FIntVector &VoxelCoord) {
  ManagedVoxels.Remove(VoxelCoord);
}

bool UEntityChunk::IsEmpty() const { return ManagedVoxels.IsEmpty(); }