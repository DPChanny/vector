#include "Managers/EntityManager.h"

#include "Actors/VoxelWorldActor.h"
#include "DataAssets/VoxelEntityDataAsset.h"
#include "EntityChunk.h"
#include "Managers/DataManager.h"

void UEntityManager::Initialize() {
  if (const TObjectPtr<AVoxelWorldActor> VoxelWorld =
          Cast<AVoxelWorldActor>(GetOuter())) {
    DataManager = VoxelWorld->GetDataManager();
  }
}

void UEntityManager::Tick(const float DeltaTime) {
  for (const TObjectPtr Chunk : EntityChunks) {
    if (Chunk) {
      Chunk->Tick(DeltaTime);
    }
  }
}

void UEntityManager::OnEntityCreated(const FIntVector &GlobalCoord,
                                     const FVoxelEntityData *EntityData) {
  if (EntityToChunk.Contains(GlobalCoord) || !EntityData) {
    return;
  }

  UEntityChunk *TargetChunk = GetEntityChunk(GlobalCoord, EntityData);

  if (!TargetChunk) {
    TargetChunk = CreateEntityChunk(EntityData);
  }

  TargetChunk->AddVoxel(GlobalCoord);
  EntityToChunk.Add(GlobalCoord, TargetChunk);
}

void UEntityManager::OnEntityDestroyed(const FIntVector &GlobalCoord) {
  if (!EntityToChunk.Contains(GlobalCoord)) {
    return;
  }

  UEntityChunk *OwningChunk = EntityToChunk.FindAndRemoveChecked(GlobalCoord);
  OwningChunk->RemoveVoxel(GlobalCoord);

  if (OwningChunk->IsEmpty()) {
    EntityChunks.Remove(OwningChunk);
  } else {
    RebuildChunksAround(GlobalCoord);
  }
}

void UEntityManager::OnEntityModified(const FIntVector &GlobalCoord) {}

void UEntityManager::UpdateVoxelChunkMapping(const FIntVector &GlobalCoord,
                                             UEntityChunk *Chunk) {
  EntityToChunk.Add(GlobalCoord, Chunk);
}

UEntityChunk *
UEntityManager::GetEntityChunk(const FIntVector &GlobalCoord,
                               const FVoxelEntityData *EntityData) {
  const FIntVector NeighborOffsets[] = {
      FIntVector(1, 0, 0),  FIntVector(-1, 0, 0), FIntVector(0, 1, 0),
      FIntVector(0, -1, 0), FIntVector(0, 0, 1),  FIntVector(0, 0, -1)};

  for (const FIntVector &NeighborOffset : NeighborOffsets) {
    if (TObjectPtr<UEntityChunk> *FoundChunk =
            EntityToChunk.Find(GlobalCoord + NeighborOffset)) {
      if (const FVoxelBaseData *NeighborVoxelData =
              DataManager->GetVoxelData(GlobalCoord + NeighborOffset)) {
        if (const FVoxelEntityData *NeighborEntityData =
                dynamic_cast<const FVoxelEntityData *>(NeighborVoxelData)) {
          if (EntityData->IsIdentical(NeighborEntityData)) {
            return *FoundChunk;
          }
        }
      }
    }
  }

  return nullptr;
}

UEntityChunk *
UEntityManager::CreateEntityChunk(const FVoxelEntityData *EntityData) {
  UEntityChunk *NewChunk = nullptr;

  if (const UVoxelEntityDataAsset *EntityDataAsset =
          EntityData->GetEntityDataAsset()) {
    if (EntityDataAsset->EntityChunkClass) {
      NewChunk =
          NewObject<UEntityChunk>(this, EntityDataAsset->EntityChunkClass);
    }
  }

  if (!NewChunk) {
    NewChunk = NewObject<UEntityChunk>(this);
  }

  EntityChunks.Add(NewChunk);

  return NewChunk;
}

void UEntityManager::RebuildChunksAround(const FIntVector &GlobalCoord) {
  // 이 함수는 복셀 파괴로 인해 하나의 청크가 여러 개로 나뉘었을 때를
  // 처리합니다.
  // 1. 파괴된 위치 주변의 모든 인접 엔티티 복셀을 찾습니다.
  // 2. 각 인접 복셀에 대해 Flood Fill (or BFS/DFS) 탐색을 시작하여 연결된 복셀
  // 그룹(새로운 청크)을 찾습니다.
  // 3. 이미 방문한 복셀은 건너뛰어 중복 계산을 방지합니다.
  // 4. 각 그룹에 대해 새로운 UEntityChunk를 생성하고 관리 목록에 추가합니다.
  // (구현이 복잡하므로 여기서는 개념만 설명합니다.)
}
