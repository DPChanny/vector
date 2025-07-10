#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelChunk.generated.h"

class UProceduralMeshComponent;
class AVoxelWorld;

UCLASS()
class VECTOR_API AVoxelChunk : public AActor {
  GENERATED_BODY()

 public:
  AVoxelChunk();

  void Initialize(const FIntVector& InChunkCoord);
  void UpdateMesh() const;

 protected:
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
  TObjectPtr<UMaterialInterface> Material;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
  TObjectPtr<UProceduralMeshComponent> Mesh;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
  TObjectPtr<AVoxelWorld> World;

 private:
  FIntVector ChunkCoord;

  static FVector RoundVector(const FVector& InVector, float Precision);
  static FVector2D CalculateUV(const FVector& Position);
  FVector InterpolateVertex(const FVector& P1, const FVector& P2, float Val1,
                            float Val2) const;
};