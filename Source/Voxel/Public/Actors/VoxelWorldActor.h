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
  void InitializeNexuses(int32 NexusCount,
                         TSet<TObjectPtr<APlayerStart>>& OutPlayerStart);

 private:
  AVoxelWorldActor();

  UPROPERTY(EditDefaultsOnly, Category = "Nexus")
  float NexusRadius = 200.f;

  UPROPERTY(EditDefaultsOnly, Category = "Nexus")
  float NexusPlacementRadius = 1000.f;

  UPROPERTY(VisibleAnywhere, Category = "Managers")
  TObjectPtr<UDataManager> DataManager;
  UPROPERTY(VisibleAnywhere, Category = "Managers")
  TObjectPtr<UEntityManager> EntityManager;
  UPROPERTY(VisibleAnywhere, Category = "Managers")
  TObjectPtr<UDebugManager> DebugManager;
  UPROPERTY(VisibleAnywhere, Category = "Managers")
  TObjectPtr<UBuildManager> BuildManager;
  UPROPERTY(VisibleAnywhere, Category = "Managers")
  TObjectPtr<UMeshManager> MeshManager;

  TArray<FNexus> Nexuses;
};
