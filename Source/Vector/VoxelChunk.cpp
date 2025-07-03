#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "ProceduralMeshComponent.h"
#include "VoxelBaseDataAsset.h"
#include "VoxelBlockDataAsset.h"
#include "VoxelSubstanceDataAsset.h"
#include "Materials/MaterialInterface.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "MarchingCubesTables.h"


AVoxelChunk::AVoxelChunk()
{
	PrimaryActorTick.bCanEverTick = false;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	RootComponent = ProceduralMesh;

	ProceduralMesh->bUseAsyncCooking = true;
	ProceduralMesh->SetCastShadow(true);
	ProceduralMesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void AVoxelChunk::Initialize(AVoxelWorld* InOwningWorld, const FIntVector& InChunkCoord)
{
	OwningWorld = InOwningWorld;
	ChunkCoord = InChunkCoord;
}

FVector AVoxelChunk::InterpolateVertex(const FVector& p1, const FVector& p2, float val1, float val2) const
{
	if (FMath::IsNearlyEqual(val1, val2)) return p1;
	const float Mu = (SurfaceLevel - val1) / (val2 - val1);
	return p1 + Mu * (p2 - p1);
}

void AVoxelChunk::GenerateMesh()
{
	if (!OwningWorld) return;

	struct FMeshSectionData
	{
		TArray<FVector> Vertices;
		TArray<int32> Triangles;
		TArray<FVector> Normals;
		TArray<FVector2D> UVs;
	};
	TMap<int32, FMeshSectionData> MeshSections;

	for (int32 x = 0; x < CHUNK_SIZE; ++x)
	{
		for (int32 y = 0; y < CHUNK_SIZE; ++y)
		{
			for (int32 z = 0; z < CHUNK_SIZE; ++z)
			{
				const FIntVector LocalVoxelCoord = FIntVector(x, y, z);
				TMap<int32, int32> SubstanceCount;

				float CornerDensities[8]{};
				FVector CornerPositions[8];
				int32 CubeIndex = 0;
				for (int i = 0; i < 8; ++i) {
					const FIntVector VoxelCoord = (ChunkCoord * CHUNK_SIZE) + LocalVoxelCoord + VoxelCellCorners[i];

					CornerDensities[i] = OwningWorld->CalculateDensity(VoxelCoord);
					CornerPositions[i] = (FVector(VoxelCoord) * VOXEL_SIZE) - GetActorLocation();

					if (CornerDensities[i] > SurfaceLevel) {
						CubeIndex |= (1 << i);
						if (OwningWorld->GetVoxelID(VoxelCoord) != OwningWorld->GetVoidID())
							SubstanceCount.FindOrAdd(OwningWorld->GetVoxelID(VoxelCoord))++;
					}
				}

				if (EdgeTable[CubeIndex] == 0) continue;

				int32 VoxelID = OwningWorld->GetVoidID();
				int32 MaxCount = 0;

				for (const auto& Pair : SubstanceCount)
				{
					if (Pair.Value > MaxCount)
					{
						MaxCount = Pair.Value;
						VoxelID = Pair.Key;
					}
				}

				FVector EdgeVertices[12];

				for (int i = 0; i < 12; ++i)
				{
					if ((EdgeTable[CubeIndex] & (1 << i)))
					{
						const int32 CornerA = EdgeConnections[i][0];
						const int32 CornerB = EdgeConnections[i][1];
						EdgeVertices[i] = InterpolateVertex(CornerPositions[CornerA], CornerPositions[CornerB], CornerDensities[CornerA], CornerDensities[CornerB]);
					}
				}

				FMeshSectionData& Section = MeshSections.FindOrAdd(VoxelID);
				for (int i = 0; TriTable[CubeIndex][i] != -1; i += 3)
				{
					const int32 VertCount = Section.Vertices.Num();

					const FVector V0 = EdgeVertices[TriTable[CubeIndex][i]];
					const FVector V1 = EdgeVertices[TriTable[CubeIndex][i + 1]];
					const FVector V2 = EdgeVertices[TriTable[CubeIndex][i + 2]];

					Section.Vertices.Add(V0);
					Section.Vertices.Add(V1);
					Section.Vertices.Add(V2);

					Section.Triangles.Add(VertCount);
					Section.Triangles.Add(VertCount + 1);
					Section.Triangles.Add(VertCount + 2);
					
					const FVector FlatNormal = -FVector::CrossProduct(V1 - V0, V2 - V0).GetSafeNormal();

					Section.Normals.Add(FlatNormal);
					Section.Normals.Add(FlatNormal);
					Section.Normals.Add(FlatNormal);

					Section.UVs.Append({ FVector2D::ZeroVector, FVector2D::ZeroVector, FVector2D::ZeroVector });
				}
			}
		}
	}

	ProceduralMesh->ClearAllMeshSections();
	for (const auto& Pair : MeshSections)
	{
		const int32 VoxelID = Pair.Key;
		const FMeshSectionData& SectionData = Pair.Value;

		ProceduralMesh->CreateMeshSection(VoxelID, SectionData.Vertices, SectionData.Triangles, SectionData.Normals, SectionData.UVs, TArray<FColor>(), TArray<FProcMeshTangent>(), true);

		if (UMaterialInterface* VoxelMaterial = OwningWorld->GetVoxelMaterial(VoxelID))
		{
			ProceduralMesh->SetMaterial(VoxelID, VoxelMaterial);
		}
	}
}

void AVoxelChunk::DrawDebugBounds(float Thickness, FColor Color)
{
	if (!GetWorld()) return;

	FVector ChunkWorldOrigin = FVector(ChunkCoord) * CHUNK_SIZE * VOXEL_SIZE;
	FVector ChunkWorldSize = FVector(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE) * VOXEL_SIZE;

	DrawDebugBox(
		GetWorld(),
		ChunkWorldOrigin + ChunkWorldSize * 0.5f,
		ChunkWorldSize * 0.5f,
		FRotator::ZeroRotator.Quaternion(),
		Color,
		true,
		-1.f,
		0,
		Thickness
	);
}