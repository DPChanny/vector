#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelChunk.generated.h"

class UProceduralMeshComponent;
class AVoxelWorld;

UCLASS()
class VECTOR_API AVoxelChunk : public AActor
{
	GENERATED_BODY()

public:
	AVoxelChunk();

	void Initialize(const FIntVector& InChunkCoord);
	void UpdateMesh();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMaterialInterface> Material;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProceduralMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AVoxelWorld> World;

private:
	FIntVector ChunkCoord;

	FVector InterpolateVertex(const FVector& p1, const FVector& p2, float val1, float val2) const;
};