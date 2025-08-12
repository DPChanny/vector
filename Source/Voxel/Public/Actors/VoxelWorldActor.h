#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorldActor.generated.h"

class UEntityManager;
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

  UPROPERTY(VisibleAnywhere) FVector Center = FVector::ZeroVector;
  UPROPERTY(VisibleAnywhere) float Radius = 0.f;
};

UCLASS()

class VOXEL_API AVoxelWorldActor : public AActor {
  GENERATED_BODY()

 public:
  const TObjectPtr<UDataManager>& GetDataManager() const { return DataManager; }

  const TObjectPtr<UMeshManager>& GetMeshManager() const { return MeshManager; }

  const TObjectPtr<UDebugManager>& GetDebugManager() const {
    return DebugManager;
  }

  const TObjectPtr<UBuildManager>& GetBuildManager() const {
    return BuildManager;
  }

  const TObjectPtr<UEntityManager>& GetEntityManager() const {
    return EntityManager;
  }

  void Initialize(int32 NumberOfPlayers);

  const TArray<TObjectPtr<APlayerStart>>& GetPlayerStarts() const {
    return PlayerStarts;
  }

 private:
  void InitializeNexuses(int32 NexusCount);

  virtual void Tick(float DeltaSeconds) override;

  AVoxelWorldActor();

  UPROPERTY(EditDefaultsOnly, Category = "World | World")
  int32 ChunkSize = 32;

  UPROPERTY(EditDefaultsOnly, Category = "World | World")
  int32 VoxelSize = 50;

  UPROPERTY(EditDefaultsOnly, Category = "World | Data")
  TObjectPtr<UVoxelBlockDataAsset> VoxelDefaultBlockDataAsset;

  UPROPERTY(EditDefaultsOnly, Category = "World | Chunk")
  TSubclassOf<AVoxelChunkActor> VoxelChunkActorClass;

  UPROPERTY(EditDefaultsOnly, Category = "World | Nexus")
  float NexusRadius = 200.f;

  UPROPERTY(EditDefaultsOnly, Category = "World | Nexus")
  float NexusPlacementRadius = 1000.f;

  UPROPERTY(EditDefaultsOnly, Category = "World | Debug")
  TSubclassOf<AVoxelDebugActor> VoxelDebugActorClass;

  UPROPERTY()
  TArray<TObjectPtr<APlayerStart>> PlayerStarts;

  UPROPERTY()
  TObjectPtr<UDataManager> DataManager;

  UPROPERTY()
  TObjectPtr<UEntityManager> EntityManager;

  UPROPERTY()
  TObjectPtr<UDebugManager> DebugManager;

  UPROPERTY()
  TObjectPtr<UBuildManager> BuildManager;

  UPROPERTY()
  TObjectPtr<UMeshManager> MeshManager;

  TArray<FNexus> Nexuses;
};
