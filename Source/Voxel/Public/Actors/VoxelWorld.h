#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorld.generated.h"

class UVoxelBorderDataAsset;
class UVoxelVoidDataAsset;
class AVoxelDebugActor;
class UVoxelBlockDataAsset;
class UDebugManager;
class UDataManager;
class UBuildManager;
class UMeshManager;
class AVoxelChunkActor;

USTRUCT()
struct FNexus {
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere) FVector Center;
  UPROPERTY(VisibleAnywhere) float Radius;
};

UCLASS()
class VOXEL_API AVoxelWorld : public AActor {
  GENERATED_BODY()

public:
  const TObjectPtr<UDataManager> &GetDataManager() const { return DataManager; }
  const TObjectPtr<UMeshManager> &GetMeshManager() const { return MeshManager; }
  const TObjectPtr<UDebugManager> &GetDebugManager() const {
    return DebugManager;
  }
  const TObjectPtr<UBuildManager> &GetBuildManager() const {
    return BuildManager;
  }

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
  TObjectPtr<UVoxelBlockDataAsset> VoxelDefaultBlockDataAsset;

  UPROPERTY(EditDefaultsOnly, Category = "Data")
  TObjectPtr<UVoxelBorderDataAsset> VoxelBorderDataAsset;

  UPROPERTY(EditDefaultsOnly, Category = "Chunk")
  TSubclassOf<AVoxelChunkActor> VoxelChunkActor;

  UPROPERTY(EditDefaultsOnly, Category = "Nexus")
  float NexusRadius = 200.f;

  UPROPERTY(EditDefaultsOnly, Category = "Debug")
  TSubclassOf<AVoxelDebugActor> VoxelDebugActor;

  UPROPERTY()
  TArray<TObjectPtr<APlayerStart>> PlayerStarts;

  UPROPERTY()
  TObjectPtr<UDataManager> DataManager;

  UPROPERTY()
  TObjectPtr<UDebugManager> DebugManager;

  UPROPERTY()
  TObjectPtr<UBuildManager> BuildManager;

  UPROPERTY()
  TObjectPtr<UMeshManager> MeshManager;

  TArray<FNexus> Nexuses;
};
