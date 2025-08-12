#include "Managers/DataManager.h"

#include "Actors/VoxelChunkActor.h"
#include "DataAssets/VoxelBaseDataAsset.h"
#include "DataAssets/VoxelEntityDataAsset.h"
#include "Managers/DebugManager.h"
#include "Managers/EntityManager.h"
#include "Managers/MeshManager.h"

void UDataManager::InitializeComponent() {
  Super::InitializeComponent();

  if (const AActor* Owner = GetOwner()) {
    DebugManager = Owner->GetComponentByClass<UDebugManager>();
    MeshManager = Owner->GetComponentByClass<UMeshManager>();
    EntityManager = Owner->GetComponentByClass<UEntityManager>();
  }
}

FVoxelChunk* UDataManager::LoadVoxelChunk(const FIntVector& ChunkCoord) {
  if (VoxelChunks.Contains(ChunkCoord)) {
    return VoxelChunks.Find(ChunkCoord);
  }

  if (!VoxelChunkActorClass) {
    return nullptr;
  }

  FVoxelChunk NewVoxelChunk(
      ChunkSize, VoxelDefaultBlockDataAsset,
      FVoxelBlockParams(VoxelDefaultBlockDataAsset->MaxDurability));

  UWorld* World = GetWorld();
  if (!World) {
    return nullptr;
  }

  FActorSpawnParameters SpawnParams;
  SpawnParams.Owner = Cast<AActor>(GetOuter());

  NewVoxelChunk.VoxelChunkActor = World->SpawnActor<AVoxelChunkActor>(
      VoxelChunkActorClass, GlobalToWorldCoord(ChunkToGlobalCoord(ChunkCoord)),
      FRotator::ZeroRotator, SpawnParams);

  if (AActor* Owner = Cast<AActor>(GetOuter())) {
    NewVoxelChunk.VoxelChunkActor->AttachToActor(
        Owner, FAttachmentTransformRules::KeepWorldTransform);
  }
  NewVoxelChunk.VoxelChunkActor->Initialize(ChunkCoord);

  VoxelChunks.Add(ChunkCoord, MoveTemp(NewVoxelChunk));

  return VoxelChunks.Find(ChunkCoord);
}

void UDataManager::UnloadVoxelChunk(const FIntVector& ChunkCoord) {
  if (const FVoxelChunk* VoxelChunk = VoxelChunks.Find(ChunkCoord)) {
    if (VoxelChunk->VoxelChunkActor) {
      VoxelChunk->VoxelChunkActor->Destroy();
    }
    VoxelChunks.Remove(ChunkCoord);
  }
}

FVoxelChunk* UDataManager::GetVoxelChunk(const FIntVector& ChunkCoord) {
  if (FVoxelChunk* VoxelChunk = VoxelChunks.Find(ChunkCoord)) {
    return VoxelChunk;
  }
  return LoadVoxelChunk(ChunkCoord);
}

const FVoxelBaseData* UDataManager::GetVoxelData(
    const FIntVector& GlobalCoord) {
  if (const FVoxelChunk* VoxelChunk =
          GetVoxelChunk(GlobalToChunkCoord(GlobalCoord))) {
    return VoxelChunk->GetVoxelData(GlobalCoordToIndex(GlobalCoord));
  }
  return nullptr;
}

void UDataManager::ModifyVoxelData(
    const FIntVector& GlobalCoord,
    const TFunction<void(FVoxelBaseData*)>& Modifier, const bool bAutoDebug) {
  if (const FVoxelChunk* VoxelChunk =
          GetVoxelChunk(GlobalToChunkCoord(GlobalCoord))) {
    Modifier(VoxelChunk->GetVoxelData(GlobalCoordToIndex(GlobalCoord)));

    if (EntityManager) {
      if (const FVoxelEntityData* EntityData =
              dynamic_cast<const FVoxelEntityData*>(
                  VoxelChunk->GetVoxelData(GlobalCoordToIndex(GlobalCoord)))) {
        EntityManager->OnEntityModified(GlobalCoord, *EntityData);
      }
    }

    if (DebugManager && bAutoDebug) {
      DebugManager->SetDebugVoxel(GlobalCoord);
    }

    if (MeshManager) {
      MeshManager->SetDirtyChunk(GlobalCoord);
    }
  }
}

void UDataManager::SetVoxelData(const FIntVector& GlobalCoord,
                                FVoxelBaseData* NewVoxelData,
                                const bool bAutoDebug) {
  if (const FVoxelChunk* VoxelChunk =
          GetVoxelChunk(GlobalToChunkCoord(GlobalCoord))) {

    const bool bWasEntity =
        dynamic_cast<const FVoxelEntityData*>(VoxelChunk->GetVoxelData(
            GlobalCoordToIndex(GlobalCoord))) != nullptr;

    VoxelChunk->SetVoxelData(GlobalCoordToIndex(GlobalCoord), NewVoxelData);

    if (DebugManager && bAutoDebug) {
      DebugManager->SetDebugVoxel(GlobalCoord);
    }
    if (MeshManager) {
      MeshManager->SetDirtyChunk(GlobalCoord);
    }
    if (EntityManager) {
      if (bWasEntity || dynamic_cast<const FVoxelEntityData*>(NewVoxelData)) {
        EntityManager->SetDirtyEntity(GlobalCoord);
      }
    }
  }
}

FIntVector UDataManager::GlobalToChunkCoord(
    const FIntVector& GlobalCoord) const {
  return FIntVector(
      FMath::FloorToInt(static_cast<float>(GlobalCoord.X) / ChunkSize),
      FMath::FloorToInt(static_cast<float>(GlobalCoord.Y) / ChunkSize),
      FMath::FloorToInt(static_cast<float>(GlobalCoord.Z) / ChunkSize));
}

FIntVector UDataManager::ChunkToGlobalCoord(
    const FIntVector& ChunkCoord) const {
  return ChunkCoord * ChunkSize;
}

FIntVector UDataManager::GlobalToLocalCoord(
    const FIntVector& GlobalCoord) const {
  return FIntVector((GlobalCoord.X % ChunkSize + ChunkSize) % ChunkSize,
                    (GlobalCoord.Y % ChunkSize + ChunkSize) % ChunkSize,
                    (GlobalCoord.Z % ChunkSize + ChunkSize) % ChunkSize);
}

int32 UDataManager::GlobalCoordToIndex(const FIntVector& GlobalCoord) const {
  return LocalCoordToIndex(GlobalToLocalCoord(GlobalCoord));
}

UDataManager::UDataManager() {
  bWantsInitializeComponent = true;
}

FIntVector UDataManager::LocalToGlobalCoord(
    const FIntVector& LocalCoord, const FIntVector& ChunkCoord) const {
  return ChunkToGlobalCoord(ChunkCoord) + LocalCoord;
}

FIntVector UDataManager::WorldToGlobalCoord(const FVector& WorldCoord) const {
  return FIntVector(FMath::RoundToInt(WorldCoord.X / VoxelSize),
                    FMath::RoundToInt(WorldCoord.Y / VoxelSize),
                    FMath::RoundToInt(WorldCoord.Z / VoxelSize));
}

FVector UDataManager::GlobalToWorldCoord(const FIntVector& GlobalCoord) const {
  return FVector(GlobalCoord) * VoxelSize;
}

int32 UDataManager::LocalCoordToIndex(const FIntVector& LocalCoord) const {
  return LocalCoord.X + LocalCoord.Y * ChunkSize +
         LocalCoord.Z * ChunkSize * ChunkSize;
}