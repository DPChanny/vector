#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "ProceduralMeshComponent.h"
#include "VoxelBaseDataAsset.h"
#include "VoxelSubstanceDataAsset.h"
#include "VoxelBlockDataAsset.h"
#include "Materials/MaterialInterface.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "MarchingCubesTables.h"
#include <Kismet/GameplayStatics.h>


AVoxelChunk::AVoxelChunk()
	: ChunkCoord(FIntVector::ZeroValue)
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	Mesh->bUseAsyncCooking = true;
	Mesh->SetCastShadow(true);
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void AVoxelChunk::Initialize(const FIntVector& InChunkCoord)
{
	World = Cast<AVoxelWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), AVoxelWorld::StaticClass()));
	ChunkCoord = InChunkCoord;

	UpdateMesh();
}

FVector AVoxelChunk::InterpolateVertex(const FVector& p1, const FVector& p2, float val1, float val2) const
{
	if (FMath::IsNearlyEqual(val1, val2)) return p1;
	const float Mu = (World->GetSurfaceLevel() - val1) / (val2 - val1);
	return p1 + Mu * (p2 - p1);
}

void AVoxelChunk::UpdateMesh()
{
	if (!World) return;

	struct FMeshSectionData
	{
		TArray<FVector> Vertices;
		TArray<int32> Triangles;
		TArray<FVector> Normals;
		TArray<FVector2D> UVs;
	};
	TMap<int32, FMeshSectionData> MeshSections;

	const int32 ChunkSize = World->GetChunkSize();
	const int32 VoxelSize = World->GetVoxelSize();

	for (int32 x = 0; x < ChunkSize; ++x)
	{
		for (int32 y = 0; y < ChunkSize; ++y)
		{
			for (int32 z = 0; z < ChunkSize; ++z)
			{
				const FIntVector LocalVoxelCoord = FIntVector(x, y, z);
				TMap<int32, int32> SubstanceCount;

				float CornerDensities[8]{};
				FVector CornerPositions[8];
				int32 CubeIndex = 0;
				for (int i = 0; i < 8; ++i) {
					const FIntVector VoxelCoord = (ChunkCoord * ChunkSize) + LocalVoxelCoord + VoxelCellCorners[i];

					CornerDensities[i] = World->GetDensity(VoxelCoord);
					CornerPositions[i] = (FVector(VoxelCoord) * VoxelSize) - GetActorLocation();

					if (CornerDensities[i] > World->GetSurfaceLevel()) {
						CubeIndex |= (1 << i);
						if (const UVoxelBaseDataAsset* VoxelData = World->GetVoxelData(World->GetVoxelID(VoxelCoord)))
							if (Cast<UVoxelSubstanceDataAsset>(VoxelData))
								SubstanceCount.FindOrAdd(World->GetVoxelID(VoxelCoord))++;
					}
				}

				if (EdgeTable[CubeIndex] == 0) continue;

				int32 VoxelID = World->GetDefaultBlockID();
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

	Mesh->ClearAllMeshSections();
	for (const auto& Pair : MeshSections)
	{
		const int32 VoxelID = Pair.Key;
		const FMeshSectionData& SectionData = Pair.Value;

		Mesh->CreateMeshSection(VoxelID, SectionData.Vertices, SectionData.Triangles, SectionData.Normals, SectionData.UVs, TArray<FColor>(), TArray<FProcMeshTangent>(), true);

		if (const UVoxelBaseDataAsset* VoxelData = World->GetVoxelData(VoxelID))
			if (const UVoxelSubstanceDataAsset* SubstanceData = Cast<UVoxelSubstanceDataAsset>(VoxelData))
				Mesh->SetMaterial(VoxelID, SubstanceData->Material);
	}
}
