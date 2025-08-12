#pragma once

#include "CoreMinimal.h"
#include "Damageable.h"
#include "GameFramework/Actor.h"
#include "VoxelChunkActor.generated.h"

class UBuildManager;
class UDebugManager;
class UProceduralMeshComponent;
class UDataManager;

UCLASS()

class VOXEL_API AVoxelChunkActor : public AActor, public IDamageable {
  GENERATED_BODY()

 public:
  AVoxelChunkActor();

  void Initialize(const FIntVector& InChunkCoord);
  void UpdateMesh() const;

  virtual void OnDamage_Implementation(const FVector HitPoint,
                                       const float DamageAmount,
                                       const float DamageRange) override;

 private:
  UPROPERTY(EditDefaultsOnly)
  TObjectPtr<UMaterialInterface> Material;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UProceduralMeshComponent> Mesh;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UDataManager> DataManager;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<const UBuildManager> BuildManager;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UDebugManager> DebugManager;

  UPROPERTY(VisibleAnywhere)
  FIntVector ChunkCoord;

  static FVector RoundVector(const FVector& InVector, float Precision);
  static FVector2D GetUV(const FVector& Position);
  static FVector InterpolateVertex(const FVector& P1, const FVector& P2,
                                   float Val1, float Val2);
};