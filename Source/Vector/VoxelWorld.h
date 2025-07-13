#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorld.generated.h"

class AVoxelDebugActor;
class APlayerStart;
class UMaterialInterface;
class UVoxelBlockDataAsset;
class UVoxelVoidDataAsset;
class UVoxelBorderDataAsset;
class UVoxelDebug;
class UVoxelData;
class AVoxelChunkActor;

USTRUCT()
struct FNexus {
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere) FVector Center;
  UPROPERTY(VisibleAnywhere) float Radius;
};

UCLASS()
class VECTOR_API AVoxelWorld : public AActor {
  GENERATED_BODY()

public:
  const TObjectPtr<UVoxelData> &GetVoxelData() const { return VoxelData; }
  const TObjectPtr<UVoxelDebug> &GetVoxelDebug() const { return VoxelDebug; }

  void Initialize(int32 NumberOfPlayers);

  const TArray<TObjectPtr<APlayerStart>> &GetPlayerStarts() const {
    return PlayerStarts;
  }

  void DamageVoxel(const FVector &Center, float Radius,
                   float DamageAmount) const;
  void ConstructVoxel(const FVector &Center, float Radius,
                      float ConstructionAmount, int32 VoxelIDToConstruct) const;

private:
  void InitializeNexuses(int32 NexusCount);

  void GetGlobalCoordsInRadius(const FVector &Center, float Radius,
                               TSet<FIntVector> &FoundGlobalCoords) const;

  AVoxelWorld();

  UPROPERTY(EditDefaultsOnly, Category = "World")
  int32 ChunkSize = 10;

  UPROPERTY(EditDefaultsOnly, Category = "World")
  int32 VoxelSize = 50;

  UPROPERTY(EditDefaultsOnly, Category = "World")
  FIntVector WorldSizeInChunks = FIntVector(20, 20, 20);

  UPROPERTY(EditDefaultsOnly, Category = "Data")
  TArray<TObjectPtr<UVoxelBlockDataAsset>> VoxelBlockDataAssets;

  UPROPERTY(EditDefaultsOnly, Category = "Data")
  TObjectPtr<UVoxelVoidDataAsset> VoxelVoidDataAsset;

  UPROPERTY(EditDefaultsOnly, Category = "Data")
  TObjectPtr<UVoxelBorderDataAsset> VoxelBorderDataAsset;

  UPROPERTY(EditDefaultsOnly, Category = "Chunk")
  TSubclassOf<AVoxelChunkActor> VoxelChunk;

  UPROPERTY(EditDefaultsOnly, Category = "Nexus")
  float NexusRadius = 200.f;

  UPROPERTY(EditDefaultsOnly, Category = "Debug")
  TSubclassOf<AVoxelDebugActor> VoxelDebugActor;

  void
  ProcessVoxel(const FVector &Center, float Radius,
               const TFunction<void(const FIntVector &, TSet<FIntVector> &)>
                   &VoxelModifier) const;
  void AddDirtyChunk(const FIntVector &GlobalCoord,
                     TSet<FIntVector> &DirtyChunkCoords) const;
  void UpdateDirtyChunk(const TSet<FIntVector> &DirtyChunkCoords) const;
  bool IsSurfaceVoxel(const FIntVector &VoxelCoord) const;

  UPROPERTY()
  TArray<TObjectPtr<APlayerStart>> PlayerStarts;

  UPROPERTY()
  TObjectPtr<UVoxelData> VoxelData;

  UPROPERTY()
  TObjectPtr<UVoxelDebug> VoxelDebug;

  TArray<FNexus> Nexuses;
};
