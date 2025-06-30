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

	const FVector ChunkActorWorldOrigin = FVector(ChunkCoord) * CHUNK_SIZE * VOXEL_SIZE;

	for (int32 x = 0; x < CHUNK_SIZE; ++x)
	{
		for (int32 y = 0; y < CHUNK_SIZE; ++y)
		{
			for (int32 z = 0; z < CHUNK_SIZE; ++z)
			{
				float CornerDensities[8];
				FVector CornerPositions[8];
				int32 CubeIndex = 0;
				for (int i = 0; i < 8; ++i)
				{
					const FIntVector GlobalGridCoord = (ChunkCoord * CHUNK_SIZE) + FIntVector(x, y, z) + VoxelCellCorners[i];

					CornerDensities[i] = OwningWorld->CalculateGlobalDensity(GlobalGridCoord);
					CornerPositions[i] = (FVector(GlobalGridCoord) * VOXEL_SIZE) - ChunkActorWorldOrigin;

					if (CornerDensities[i] > SurfaceLevel) CubeIndex |= (1 << i);
				}

				if (EdgeTable[CubeIndex] == 0) continue;

				int32 VoxelID = OwningWorld->GetDefaultID();

				bool bMaterialFound = false;
				for (int i = 0; i < 8; ++i)
				{
					if (CornerDensities[i] > SurfaceLevel)
					{
						const FIntVector CornerGlobalBlockCoord = (ChunkCoord * CHUNK_SIZE) + FIntVector(x, y, z) + VoxelCellCorners[i];
						const int32 CornerVoxelID = OwningWorld->GetGlobalVoxelID(CornerGlobalBlockCoord);

						if (CornerVoxelID != OwningWorld->GetVoidID())
						{
							VoxelID = CornerVoxelID;
							bMaterialFound = true;
							break;
						}
					}
				}

				if (VoxelID == OwningWorld->GetVoidID()) continue;


				FVector EdgeVertices[12];
				// 각 EdgeVertex에 해당하는 GlobalGridCoord를 저장할 배열
				FIntVector EdgeVertexGlobalGridCoords[12];

				for (int i = 0; i < 12; ++i)
				{
					if ((EdgeTable[CubeIndex] & (1 << i)))
					{
						const int32 CornerA = EdgeConnections[i][0];
						const int32 CornerB = EdgeConnections[i][1];
						EdgeVertices[i] = InterpolateVertex(CornerPositions[CornerA], CornerPositions[CornerB], CornerDensities[CornerA], CornerDensities[CornerB]);

						// EdgeVertex의 GlobalGridCoord를 계산 (보간된 위치를 그리드 좌표로 변환)
						// EdgeVertex는 로컬 공간에 있으므로, ChunkActorWorldOrigin을 다시 더해 월드 공간으로 변환 후 VOXEL_SIZE로 나눕니다.
						EdgeVertexGlobalGridCoords[i] = FIntVector((EdgeVertices[i] + ChunkActorWorldOrigin) / VOXEL_SIZE);
					}
				}

				FMeshSectionData& Section = MeshSections.FindOrAdd(VoxelID);
				for (int i = 0; TriTable[CubeIndex][i] != -1; i += 3)
				{
					const int32 VertCount = Section.Vertices.Num();

					Section.Vertices.Add(EdgeVertices[TriTable[CubeIndex][i]]);
					Section.Vertices.Add(EdgeVertices[TriTable[CubeIndex][i + 1]]);
					Section.Vertices.Add(EdgeVertices[TriTable[CubeIndex][i + 2]]);

					Section.Triangles.Add(VertCount);
					Section.Triangles.Add(VertCount + 1);
					Section.Triangles.Add(VertCount + 2);

					FVector SmoothNormal0 = OwningWorld->GetSmoothNormalAtGlobalCoord(EdgeVertexGlobalGridCoords[TriTable[CubeIndex][i]]);
					FVector SmoothNormal1 = OwningWorld->GetSmoothNormalAtGlobalCoord(EdgeVertexGlobalGridCoords[TriTable[CubeIndex][i + 1]]);
					FVector SmoothNormal2 = OwningWorld->GetSmoothNormalAtGlobalCoord(EdgeVertexGlobalGridCoords[TriTable[CubeIndex][i + 2]]);

					Section.Normals.Add(SmoothNormal0);
					Section.Normals.Add(SmoothNormal1);
					Section.Normals.Add(SmoothNormal2);

					Section.UVs.Append({ FVector2D::ZeroVector, FVector2D::ZeroVector, FVector2D::ZeroVector });
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("%d"),
		MeshSections.Num());

	ProceduralMesh->ClearAllMeshSections();
	for (const auto& Pair : MeshSections)
	{
		const int32 VoxelID = Pair.Key;
		const FMeshSectionData& SectionData = Pair.Value;

		ProceduralMesh->CreateMeshSection(VoxelID, SectionData.Vertices, SectionData.Triangles, SectionData.Normals, SectionData.UVs, TArray<FColor>(), TArray<FProcMeshTangent>(), true);

		UE_LOG(LogTemp, Log, TEXT("%d %d %d"),
			SectionData.Vertices.Num(),
			SectionData.Triangles.Num(), SectionData.Normals.Num());

		if (UMaterialInterface* VoxelMaterial = OwningWorld->GetVoxelMaterial(VoxelID))
		{
			ProceduralMesh->SetMaterial(VoxelID, VoxelMaterial);
		}
	}
}

void AVoxelChunk::DrawDebugBounds(float Duration, float Thickness, FColor Color)
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
		Duration,
		0,
		Thickness
	);
}