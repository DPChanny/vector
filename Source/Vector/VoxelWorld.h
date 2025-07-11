#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelBaseDataAsset.h"
#include "VoxelWorld.generated.h"

class AVoxelChunk;
class AVoxelDebugActor;
class APlayerStart;
class UMaterialInterface;
class UVoxelBlockDataAsset;
class UVoxelVoidDataAsset;
class UVoxelBorderDataAsset;

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
  AVoxelWorld();

  void Initialize(int32 NumberOfPlayers);

  UVoxelBaseDataAsset *GetVoxelData(int32 VoxelID) const;

  static int32 GetVoidID() { return 0; }
  static int32 GetBorderID() { return 1; }
  static int32 GetDefaultBlockID() { return 2; }

  static float GetSurfaceLevel() { return 0.f; }

  int32 GetChunkSize() const { return ChunkSize; }
  int32 GetVoxelSize() const { return VoxelSize; }

  const TArray<APlayerStart *> &GetPlayerStarts() const { return PlayerStarts; }

  int32 GetVoxelID(const TObjectPtr<UVoxelBaseDataAsset> &VoxelDataAsset);
  int32 GetVoxelID(const FIntVector &VoxelCoord) const;
  void SetVoxelID(const FIntVector &VoxelCoord, int32 NewVoxelID);

  float GetDurability(const FIntVector &VoxelCoord) const;
  void SetDurability(const FIntVector &VoxelCoord, float NewDurability);

  float GetDensity(const FIntVector &VoxelCoord) const;

  bool IsVoxelCoordValid(const FIntVector &VoxelCoord) const;

  int32 GetIndex(const FIntVector &VoxelCoord) const;

  void DamageVoxel(const FVector &Center, float Radius, float DamageAmount);
  void ConstructVoxel(const FVector &Center, float Radius,
                      float ConstructionAmount, int32 VoxelIDToConstruct);

  FIntVector WorldPosToVoxelCoord(const FVector &WorldPos) const;
  FIntVector VoxelCoordToChunkCoord(const FIntVector &VoxelCoord) const;

  void GetVoxelCoordsInRadius(const FVector &Center, float Radius,
                              TSet<FIntVector> &FoundVoxelCoords) const;

  void InitializeNexuses(int32 NexusCount);
  void InitializeChunk(const FIntVector &ChunkCoord);

  void SetDebugVoxels(const TSet<FIntVector> &NewDebugVoxels);
  void SetDebugVoxel(const FIntVector &NewDebugVoxel);
  void FlushDebugVoxelBuffer();

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
  TSubclassOf<AVoxelDebugActor> DebugActorClass;

private:
  void AddDebugVoxel(const FIntVector &VoxelCoord);
  void RemoveDebugVoxel(const FIntVector &VoxelCoord);

  void ProcessVoxel(const FVector &Center, float Radius,
                    const TFunction<void(const FIntVector &,
                                         TSet<FIntVector> &)> &VoxelModifier);
  void AddDirtyChunk(const FIntVector &VoxelCoord,
                     TSet<FIntVector> &DirtyChunks) const;
  void UpdateDirtyChunk(const TSet<FIntVector> &DirtyChunks);
  bool IsSurfaceVoxel(const FIntVector &VoxelCoord) const;

  UPROPERTY()
  TArray<TObjectPtr<APlayerStart>> PlayerStarts;

  UPROPERTY()
  TMap<FIntVector, TObjectPtr<AVoxelChunk>> Chunks;

  UPROPERTY()
  TMap<int32, TObjectPtr<UVoxelBaseDataAsset>> VoxelDataMap;

  TArray<FNexus> Nexuses;
  TArray<int32> VoxelIDs;
  TArray<float> Durabilities;

  TMap<FIntVector, TObjectPtr<AVoxelDebugActor>> DebugVoxels;
  TSet<FIntVector> DebugVoxelsBuffer;

  int32 WorldVolume = 0;
  FIntVector WorldSize;
};
