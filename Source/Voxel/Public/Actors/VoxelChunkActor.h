#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelChunkActor.generated.h"

class UProceduralMeshComponent;
class UDataManager;

UCLASS()
class VOXEL_API AVoxelChunkActor : public AActor {
  GENERATED_BODY()

public:
  AVoxelChunkActor();

  void Initialize(const FIntVector &InChunkCoord);
  void UpdateMesh() const;

private:
  UPROPERTY(EditDefaultsOnly)
  TObjectPtr<UMaterialInterface> Material;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UProceduralMeshComponent> Mesh;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<const UDataManager> DataManager;

  UPROPERTY(VisibleAnywhere)
  FIntVector ChunkCoord;

  static FVector RoundVector(const FVector &InVector, float Precision);
  static FVector2D GetUV(const FVector &Position);
  static FVector InterpolateVertex(const FVector &P1, const FVector &P2,
                                   float Val1, float Val2);
};