#include "Managers/MeshManager.h"

#include "Actors/VoxelChunkActor.h"
#include "Actors/VoxelWorldActor.h"
#include "Managers/DataManager.h"

void UMeshManager::BeginPlay() {
  Super::BeginPlay();
  if (const AActor* Owner = GetOwner()) {
    DataManager = Owner->GetComponentByClass<UDataManager>();
  }
}

void UMeshManager::SetDirtyChunk(const FIntVector& GlobalCoord) {
  if (!DataManager) {
    return;
  }

  const FIntVector ChunkCoord = DataManager->GlobalToChunkCoord(GlobalCoord);
  const int32 ChunkSize = DataManager->GetChunkSize();

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

void UMeshManager::FlushDirtyChunks() {
  if (!DataManager) {
    return;
  }

  for (const FIntVector& DirtyChunkCoord : DirtyChunkCoords) {
    if (const FVoxelChunk* Chunk =
            DataManager->GetVoxelChunk(DirtyChunkCoord)) {
      if (Chunk->VoxelChunkActor) {
        Chunk->VoxelChunkActor->UpdateMesh();
      }
    }
  }
  DirtyChunkCoords.Empty();
}
