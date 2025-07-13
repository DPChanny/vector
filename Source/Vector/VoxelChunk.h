#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelChunk.generated.h"

class UProceduralMeshComponent;
class UVoxelData;

UCLASS()
class VECTOR_API AVoxelChunk : public AActor {
  GENERATED_BODY()

public:
  AVoxelChunk();

  void Initialize(const FIntVector &InChunkCoord, UVoxelData *InVoxelData);
  void UpdateMesh() const;

protected:
  UPROPERTY(EditDefaultsOnly)
  TObjectPtr<UMaterialInterface> Material;

  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UProceduralMeshComponent> Mesh;

  UPROPERTY()
  TObjectPtr<UVoxelData> VoxelData;

private:
  UPROPERTY(VisibleAnywhere)
  FIntVector ChunkCoord;

  static FVector RoundVector(const FVector &InVector, float Precision);
  static FVector2D GetUV(const FVector &Position);
  static FVector InterpolateVertex(const FVector &P1, const FVector &P2,
                                   float Val1, float Val2);
};