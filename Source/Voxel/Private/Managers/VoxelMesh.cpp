#include "Managers/VoxelMesh.h"

#include "Actors/VoxelChunkActor.h"
#include "Actors/VoxelWorld.h"
#include "Managers/VoxelData.h"

void UVoxelMesh::Initialize() {
  if (const TObjectPtr<AVoxelWorld> VoxelWorld =
          Cast<AVoxelWorld>(GetOuter())) {
    VoxelData = VoxelWorld->GetVoxelData();
  }
}

void UVoxelMesh::SetDirtyChunk(const FIntVector &GlobalCoord) {
  if (!VoxelData) {
    return;
  }

  const FIntVector ChunkCoord = VoxelData->GlobalToChunkCoord(GlobalCoord);
  const int32 ChunkSize = VoxelData->GetChunkSize();

  const int32 LocalX = GlobalCoord.X % ChunkSize;
  const int32 LocalY = GlobalCoord.Y % ChunkSize;
  const int32 LocalZ = GlobalCoord.Z % ChunkSize;

  const int32 LastVoxelIndexInChunk = ChunkSize - 1;

  const int32 StartX = (LocalX == 0) ? -1 : 0;
  const int32 EndX = (LocalX == LastVoxelIndexInChunk) ? 1 : 0;

  const int32 StartY = (LocalY == 0) ? -1 : 0;
  const int32 EndY = (LocalY == LastVoxelIndexInChunk) ? 1 : 0;

  const int32 StartZ = (LocalZ == 0) ? -1 : 0;
  const int32 EndZ = (LocalZ == LastVoxelIndexInChunk) ? 1 : 0;

  for (int32 i = StartX; i <= EndX; ++i) {
    for (int32 j = StartY; j <= EndY; ++j) {
      for (int32 k = StartZ; k <= EndZ; ++k) {
        DirtyChunkCoords.Add(ChunkCoord + FIntVector(i, j, k));
      }
    }
  }
}

void UVoxelMesh::FlushDirtyChunks() {
  if (!VoxelData) {
    return;
  }

  for (const FIntVector &DirtyChunkCoord : DirtyChunkCoords) {
    if (VoxelData->IsChunk(DirtyChunkCoord)) {
      if (const FVoxelChunk *Chunk = VoxelData->GetChunk(DirtyChunkCoord)) {
        if (Chunk->VoxelChunkActor) {
          Chunk->VoxelChunkActor->UpdateMesh();
        }
      }
    }
  }
  DirtyChunkCoords.Empty();
}
