#include "VoxelData.h"
#include "VoxelBaseDataAsset.h"
#include "VoxelDebug.h"

int32 UVoxelData::GetVoxelID(
    const TObjectPtr<UVoxelBaseDataAsset> &VoxelDataAsset) {
  for (const auto &Elem : VoxelDataAssets) {
    if (Elem.Value == VoxelDataAsset) {
      return Elem.Key;
    }
  }
  return GetVoidID();
}

void UVoxelData::Initialize(
    const FIntVector &InWorldSizeInChunks, const int32 InChunkSize,
    const int32 InVoxelSize, const TSubclassOf<AVoxelChunk> &InVoxelChunk,
    const TArray<TObjectPtr<UVoxelBlockDataAsset>> &InVoxelBlockDataAssets,
    const TObjectPtr<UVoxelVoidDataAsset> &InVoxelVoidDataAsset,
    const TObjectPtr<UVoxelBorderDataAsset> &InVoxelBorderDataAsset,
    UVoxelDebug *InVoxelDebug) {
  WorldSizeInChunks = InWorldSizeInChunks;
  ChunkSize = InChunkSize;
  VoxelSize = InVoxelSize;
  ChunkVolume = ChunkSize * ChunkSize * ChunkSize;
  VoxelChunk = InVoxelChunk;
  VoxelDebug = InVoxelDebug;

  VoxelDataAssets.Empty();
  if (const TObjectPtr<UVoxelBaseDataAsset> VoidDataAsset =
          Cast<UVoxelBaseDataAsset>(InVoxelVoidDataAsset)) {
    VoxelDataAssets.Add(GetVoidID(), VoidDataAsset);
  }
  if (const TObjectPtr<UVoxelBaseDataAsset> BorderDataAsset =
          Cast<UVoxelBaseDataAsset>(InVoxelBorderDataAsset)) {
    VoxelDataAssets.Add(GetBorderID(), BorderDataAsset);
  }
  for (int32 i = 0; i < InVoxelBlockDataAssets.Num(); ++i) {
    if (InVoxelBlockDataAssets[i]) {
      VoxelDataAssets.Add(i + GetDefaultBlockID(), InVoxelBlockDataAssets[i]);
    }
  }

  Chunks.Empty();
}

void UVoxelData::LoadChunk(const FIntVector &ChunkCoord) {
  if (Chunks.Contains(ChunkCoord)) {
    return;
  }

  if (!VoxelChunk) {
    return;
  }

  FChunk NewChunk;
  NewChunk.Initialize(ChunkVolume);

  UWorld *World = GetWorld();
  if (!World) {
    return;
  }

  FActorSpawnParameters SpawnParams;
  SpawnParams.Owner = Cast<AActor>(GetOuter());
  const FVector ChunkWorldLocation =
      FVector(ChunkCoord) * ChunkSize * VoxelSize;

  AVoxelChunk *NewVoxelChunk = World->SpawnActor<AVoxelChunk>(
      VoxelChunk, ChunkWorldLocation, FRotator::ZeroRotator, SpawnParams);

  if (NewVoxelChunk) {
    if (AActor *Owner = Cast<AActor>(GetOuter())) {
      NewVoxelChunk->AttachToActor(
          Owner, FAttachmentTransformRules::KeepWorldTransform);
    }
    NewVoxelChunk->Initialize(ChunkCoord, this);
    NewChunk.VoxelChunk = NewVoxelChunk;

    Chunks.Add(ChunkCoord, MoveTemp(NewChunk));
  }
}

void UVoxelData::UnloadChunk(const FIntVector &ChunkCoord) {
  if (FChunk *Chunk = Chunks.Find(ChunkCoord)) {
    if (Chunk->VoxelChunk) {
      Chunk->VoxelChunk->Destroy();
      Chunk->VoxelChunk = nullptr;
    }
    Chunks.Remove(ChunkCoord);
  }
}

FVoxel UVoxelData::GetVoxel(const FIntVector &GlobalVoxelCoord) const {
  const FIntVector ChunkCoord = GlobalToChunkCoord(GlobalVoxelCoord);
  if (const FChunk *Chunk = Chunks.Find(ChunkCoord)) {
    const FIntVector LocalCoord = GlobalToLocalCoord(GlobalVoxelCoord);
    const int32 Index = LocalCoordToIndex(LocalCoord);
    return Chunk->GetVoxel(Index);
  }
  return FVoxel(GetVoidID(), 0.f);
}

void UVoxelData::SetVoxel(const FIntVector &GlobalVoxelCoord,
                          const FVoxel &Voxel, const bool bAutoDebug) {
  const FIntVector ChunkCoord = GlobalToChunkCoord(GlobalVoxelCoord);
  if (FChunk *Chunk = Chunks.Find(ChunkCoord)) {
    const FIntVector LocalCoord = GlobalToLocalCoord(GlobalVoxelCoord);
    const int32 Index = LocalCoordToIndex(LocalCoord);
    Chunk->SetVoxel(Index, Voxel);

    if (VoxelDebug && bAutoDebug) {
      VoxelDebug->SetDebugVoxel(GlobalVoxelCoord);
    }
  }
}

int32 UVoxelData::GetVoxelID(const FIntVector &GlobalVoxelCoord) const {
  return GetVoxel(GlobalVoxelCoord).ID;
}

void UVoxelData::SetVoxelID(const FIntVector &GlobalVoxelCoord,
                            const int32 NewVoxelID) {
  FVoxel CurrentVoxel = GetVoxel(GlobalVoxelCoord);
  CurrentVoxel.ID = NewVoxelID;
  SetVoxel(GlobalVoxelCoord, CurrentVoxel);
}

float UVoxelData::GetDurability(const FIntVector &GlobalVoxelCoord) const {
  return GetVoxel(GlobalVoxelCoord).Durability;
}

void UVoxelData::SetDurability(const FIntVector &GlobalVoxelCoord,
                               const float NewDurability) {
  FVoxel CurrentVoxel = GetVoxel(GlobalVoxelCoord);
  CurrentVoxel.Durability = NewDurability;
  SetVoxel(GlobalVoxelCoord, CurrentVoxel);
}

float UVoxelData::GetDensity(const FIntVector &GlobalVoxelCoord) const {
  const FVoxel Voxel = GetVoxel(GlobalVoxelCoord);
  if (const UVoxelBaseDataAsset *VoxelAsset = GetVoxelDataAsset(Voxel.ID)) {
    const float BaseDensity = VoxelAsset->BaseDensity;
    if (const UVoxelBlockDataAsset *BlockData =
            Cast<UVoxelBlockDataAsset>(VoxelAsset)) {
      return BaseDensity * Voxel.Durability / BlockData->MaxDurability;
    }
    return BaseDensity;
  }
  return -1.f;
}

UVoxelBaseDataAsset *UVoxelData::GetVoxelDataAsset(const int32 VoxelID) const {
  if (const TObjectPtr<UVoxelBaseDataAsset> FoundData =
          VoxelDataAssets.FindRef(VoxelID)) {
    return FoundData;
  }
  return VoxelDataAssets.FindRef(GetVoidID());
}

bool UVoxelData::IsChunkLoaded(const FIntVector &ChunkCoord) const {
  return Chunks.Contains(ChunkCoord);
}

FChunk *UVoxelData::GetChunk(const FIntVector &ChunkCoord) {
  return Chunks.Find(ChunkCoord);
}

FIntVector
UVoxelData::GlobalToChunkCoord(const FIntVector &GlobalVoxelCoord) const {
  return FIntVector(
      FMath::FloorToInt(static_cast<float>(GlobalVoxelCoord.X) / ChunkSize),
      FMath::FloorToInt(static_cast<float>(GlobalVoxelCoord.Y) / ChunkSize),
      FMath::FloorToInt(static_cast<float>(GlobalVoxelCoord.Z) / ChunkSize));
}

FIntVector
UVoxelData::GlobalToLocalCoord(const FIntVector &GlobalVoxelCoord) const {
  return FIntVector(GlobalVoxelCoord.X % ChunkSize,
                    GlobalVoxelCoord.Y % ChunkSize,
                    GlobalVoxelCoord.Z % ChunkSize);
}

int32 UVoxelData::LocalCoordToIndex(const FIntVector &LocalVoxelCoord) const {
  return LocalVoxelCoord.X + (LocalVoxelCoord.Y * ChunkSize) +
         (LocalVoxelCoord.Z * ChunkSize * ChunkSize);
}