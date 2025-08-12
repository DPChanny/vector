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

  AVoxelWorldActor();

  UPROPERTY(EditDefaultsOnly, Category = "Nexus")
  float NexusRadius = 200.f;

  UPROPERTY(EditDefaultsOnly, Category = "Nexus")
  float NexusPlacementRadius = 1000.f;

  TArray<TObjectPtr<APlayerStart>> PlayerStarts;

  UPROPERTY(EditDefaultsOnly, Category = "Manager")
  TObjectPtr<UDataManager> DataManager;

  UPROPERTY(EditDefaultsOnly, Category = "Manager")
  TObjectPtr<UEntityManager> EntityManager;

  UPROPERTY(EditDefaultsOnly, Category = "Manager")
  TObjectPtr<UDebugManager> DebugManager;

  UPROPERTY(EditDefaultsOnly, Category = "Manager")
  TObjectPtr<UBuildManager> BuildManager;

  UPROPERTY(EditDefaultsOnly, Category = "Manager")
  TObjectPtr<UMeshManager> MeshManager;

  TArray<FNexus> Nexuses;
};
