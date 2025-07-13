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
class UVoxelBuild;
class UVoxelMesh;
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
  const TObjectPtr<UVoxelBuild> &GetVoxelBuild() const { return VoxelBuild; }
  const TObjectPtr<UVoxelMesh> &GetVoxelMesh() const { return VoxelMesh; }

  void Initialize(int32 NumberOfPlayers);

  const TArray<TObjectPtr<APlayerStart>> &GetPlayerStarts() const {
    return PlayerStarts;
  }

private:
  void InitializeNexuses(int32 NexusCount);

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

  UPROPERTY()
  TArray<TObjectPtr<APlayerStart>> PlayerStarts;

  UPROPERTY()
  TObjectPtr<UVoxelData> VoxelData;

  UPROPERTY()
  TObjectPtr<UVoxelDebug> VoxelDebug;

  UPROPERTY()
  TObjectPtr<UVoxelBuild> VoxelBuild;

  UPROPERTY()
  TObjectPtr<UVoxelMesh> VoxelMesh;

  TArray<FNexus> Nexuses;
};
