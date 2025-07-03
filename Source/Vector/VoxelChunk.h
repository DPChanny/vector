#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelChunk.generated.h"

class UProceduralMeshComponent;
class AVoxelWorld;

constexpr int32 CHUNK_SIZE = 10;
constexpr int32 VOXEL_SIZE = 50;

constexpr float SURFACE_LEVEL = 0;

UCLASS()
class VECTOR_API AVoxelChunk : public AActor
{
	GENERATED_BODY()

public:
	AVoxelChunk();

	void Initialize(AVoxelWorld* InOwningWorld, const FIntVector& InChunkCoord);
	void UpdateMesh();

	void DrawDebugInfo() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProceduralMeshComponent> Mesh;

	UPROPERTY()
	TObjectPtr<AVoxelWorld> World;

private:
	FIntVector ChunkCoord;

	FVector InterpolateVertex(const FVector& p1, const FVector& p2, float val1, float val2) const;
};