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
class AVoxelChunk;

USTRUCT(BlueprintType)
struct FNexus {
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector Center;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Radius;
};

UCLASS()
class VECTOR_API AVoxelWorld : public AActor {
  GENERATED_BODY()

public:
  UVoxelData *GetVoxelData() const { return VoxelData; }
  UVoxelDebug *GetVoxelDebug() const { return VoxelDebug; }

  AVoxelWorld();

  void Initialize(int32 NumberOfPlayers);

  const TArray<APlayerStart *> &GetPlayerStarts() const { return PlayerStarts; }

  void DamageVoxel(const FVector &Center, float Radius,
                   float DamageAmount) const;
  void ConstructVoxel(const FVector &Center, float Radius,
                      float ConstructionAmount, int32 VoxelIDToConstruct) const;

  FIntVector WorldPosToVoxelCoord(const FVector &WorldPos) const;
  FIntVector VoxelCoordToChunkCoord(const FIntVector &VoxelCoord) const;

  void GetVoxelCoordsInRadius(const FVector &Center, float Radius,
                              TSet<FIntVector> &FoundVoxelCoords) const;

  void InitializeNexuses(int32 NexusCount);

protected:
  UPROPERTY(EditAnywhere, Category = "World")
  int32 ChunkSize = 10;

  UPROPERTY(EditAnywhere, Category = "World")
  int32 VoxelSize = 50;

  UPROPERTY(EditAnywhere, Category = "World")
  FIntVector WorldSizeInChunks = FIntVector(20, 20, 20);

  UPROPERTY(EditAnywhere, Category = "Voxel Data")
  TArray<TObjectPtr<UVoxelBlockDataAsset>> VoxelBlockDataAssets;

  UPROPERTY(EditAnywhere, Category = "Voxel Data")
  TObjectPtr<UVoxelVoidDataAsset> VoxelVoidDataAsset;

  UPROPERTY(EditAnywhere, Category = "Voxel Data")
  TObjectPtr<UVoxelBorderDataAsset> VoxelBorderDataAsset;

  UPROPERTY(EditAnywhere, Category = "Chunk")
  TSubclassOf<AVoxelChunk> VoxelChunk;

  UPROPERTY(EditAnywhere, Category = "Nexus")
  float NexusRadius = 200.f;

  UPROPERTY(EditDefaultsOnly, Category = "Debug")
  TSubclassOf<AVoxelDebugActor> VoxelDebugActor;

private:
  void
  ProcessVoxel(const FVector &Center, float Radius,
               const TFunction<void(const FIntVector &, TSet<FIntVector> &)>
                   &VoxelModifier) const;
  void AddDirtyChunk(const FIntVector &VoxelCoord,
                     TSet<FIntVector> &DirtyChunks) const;
  void UpdateDirtyChunk(const TSet<FIntVector> &DirtyChunks) const;
  bool IsSurfaceVoxel(const FIntVector &VoxelCoord) const;

  UPROPERTY()
  TArray<TObjectPtr<APlayerStart>> PlayerStarts;

  UPROPERTY()
  TObjectPtr<UVoxelData> VoxelData;

  UPROPERTY()
  TObjectPtr<UVoxelDebug> VoxelDebug;

  TArray<FNexus> Nexuses;
};
