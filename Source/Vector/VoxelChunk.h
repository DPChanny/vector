#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelChunk.generated.h"

class UProceduralMeshComponent;
class AVoxelWorld;

constexpr int32 CHUNK_SIZE = 20;

constexpr int32 VOXEL_SIZE = 50;


UCLASS()
class VECTOR_API AVoxelChunk : public AActor
{
	GENERATED_BODY()

public:
	AVoxelChunk();
	void Initialize(AVoxelWorld* InOwningWorld, const FIntVector& InChunkCoord);

	void GenerateMesh();

	void DrawDebugBounds(float Thickness = 10.f, FColor Color = FColor::Red);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProceduralMeshComponent> ProceduralMesh;

	UPROPERTY()
	TObjectPtr<AVoxelWorld> OwningWorld;

	FIntVector ChunkCoord;

	float SurfaceLevel = 0.0f;

	FVector InterpolateVertex(const FVector& p1, const FVector& p2, float val1, float val2) const;
};