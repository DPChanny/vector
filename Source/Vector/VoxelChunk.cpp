#include "VoxelChunk.h"

#include <Kismet/GameplayStatics.h>

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "MarchingCubesTables.h"
#include "Materials/MaterialInterface.h"
#include "ProceduralMeshComponent.h"
#include "VoxelBaseDataAsset.h"
#include "VoxelBlockDataAsset.h"
#include "VoxelSubstanceDataAsset.h"
#include "VoxelWorld.h"

AVoxelChunk::AVoxelChunk() : ChunkCoord(FIntVector::ZeroValue) {
  PrimaryActorTick.bCanEverTick = false;

  Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
  RootComponent = Mesh;

  Mesh->bUseAsyncCooking = true;
  Mesh->SetCastShadow(true);
  Mesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void AVoxelChunk::Initialize(const FIntVector& InChunkCoord) {
  World = Cast<AVoxelWorld>(UGameplayStatics::GetActorOfClass(
      GetWorld(), AVoxelWorld::StaticClass()));
  ChunkCoord = InChunkCoord;

  UpdateMesh();
}

FVector AVoxelChunk::InterpolateVertex(const FVector& p1, const FVector& p2,
                                       float val1, float val2) const {
  if (FMath::IsNearlyEqual(val1, val2)) return p1;
  const float Mu = (World->GetSurfaceLevel() - val1) / (val2 - val1);
  return p1 + Mu * (p2 - p1);
}
void AVoxelChunk::UpdateMesh() {
  if (!World) return;

  TArray<FVector> Vertices;
  TArray<int32> Triangles;
  TArray<FVector> Normals;
  TArray<FLinearColor> Colors;
  TArray<FVector2D> UVs;

  TMap<FVector, int32> VertexMap;
  TMap<int32, TArray<FVector>> VertexNormals;

  const int32 ChunkSize = World->GetChunkSize();
  const int32 VoxelSize = World->GetVoxelSize();

  for (int32 x = 0; x < ChunkSize; ++x) {
    for (int32 y = 0; y < ChunkSize; ++y) {
      for (int32 z = 0; z < ChunkSize; ++z) {
        const FIntVector VoxelCoord =
            (ChunkCoord * ChunkSize) + FIntVector(x, y, z);

        float CornerDensities[8]{};
        FVector CornerPositions[8];
        int32 CubeIndex = 0;
        for (int i = 0; i < 8; ++i) {
          const FIntVector CornerVoxelCoord = VoxelCoord + CornerOffsets[i];

          CornerDensities[i] = World->GetDensity(CornerVoxelCoord);
          CornerPositions[i] =
              (FVector(CornerVoxelCoord) * VoxelSize) - GetActorLocation();

          if (CornerDensities[i] > World->GetSurfaceLevel())
            CubeIndex |= (1 << i);
        }

        if (EdgeTable[CubeIndex] == 0) continue;

        FVector EdgeVertices[12];
        FLinearColor EdgeVerticeColors[12];

        for (int i = 0; i < 12; ++i) {
          if ((EdgeTable[CubeIndex] & (1 << i))) {
            const int32 CornerA = EdgeConnections[i][0];
            const int32 CornerB = EdgeConnections[i][1];
            EdgeVertices[i] = InterpolateVertex(
                CornerPositions[CornerA], CornerPositions[CornerB],
                CornerDensities[CornerA], CornerDensities[CornerB]);
            if (const UVoxelSubstanceDataAsset* VoxelData =
                    Cast<UVoxelSubstanceDataAsset>(
                        World->GetVoxelData(World->GetVoxelID(
                            VoxelCoord +
                            (CornerDensities[CornerA] > CornerDensities[CornerB]
                                 ? CornerOffsets[CornerA]
                                 : CornerOffsets[CornerB]))))) {
              EdgeVerticeColors[i] = VoxelData->VertexColor;
            }
          }
        }

        for (int i = 0; TriTable[CubeIndex][i] != -1; i += 3) {
          const FVector V0 = EdgeVertices[TriTable[CubeIndex][i]];
          const FVector V1 = EdgeVertices[TriTable[CubeIndex][i + 1]];
          const FVector V2 = EdgeVertices[TriTable[CubeIndex][i + 2]];

          FVector RoundedV0 = RoundVector(V0, 0.001f);
          FVector RoundedV1 = RoundVector(V1, 0.001f);
          FVector RoundedV2 = RoundVector(V2, 0.001f);

          int32 VertexIndex0, VertexIndex1, VertexIndex2;

          int32* ExistingIndex = VertexMap.Find(RoundedV0);
          if (ExistingIndex) {
            VertexIndex0 = *ExistingIndex;
          } else {
            VertexIndex0 = Vertices.Add(V0);
            Normals.Add(FVector::ZeroVector);
            Colors.Add(EdgeVerticeColors[TriTable[CubeIndex][i]]);
            UVs.Add(CalculateUV(V0));
            VertexMap.Add(RoundedV0, VertexIndex0);
            VertexNormals.Add(VertexIndex0, TArray<FVector>());
          }

          ExistingIndex = VertexMap.Find(RoundedV1);
          if (ExistingIndex) {
            VertexIndex1 = *ExistingIndex;
          } else {
            VertexIndex1 = Vertices.Add(V1);
            Normals.Add(FVector::ZeroVector);
            Colors.Add(EdgeVerticeColors[TriTable[CubeIndex][i + 1]]);
            UVs.Add(CalculateUV(V1));
            VertexMap.Add(RoundedV1, VertexIndex1);
            VertexNormals.Add(VertexIndex1, TArray<FVector>());
          }

          ExistingIndex = VertexMap.Find(RoundedV2);
          if (ExistingIndex) {
            VertexIndex2 = *ExistingIndex;
          } else {
            VertexIndex2 = Vertices.Add(V2);
            Normals.Add(FVector::ZeroVector);
            Colors.Add(EdgeVerticeColors[TriTable[CubeIndex][i + 2]]);
            UVs.Add(CalculateUV(V2));
            VertexMap.Add(RoundedV2, VertexIndex2);
            VertexNormals.Add(VertexIndex2, TArray<FVector>());
          }

          Triangles.Add(VertexIndex0);
          Triangles.Add(VertexIndex1);
          Triangles.Add(VertexIndex2);

          const FVector FaceNormal =
              -FVector::CrossProduct(V1 - V0, V2 - V0).GetSafeNormal();

          VertexNormals[VertexIndex0].Add(FaceNormal);
          VertexNormals[VertexIndex1].Add(FaceNormal);
          VertexNormals[VertexIndex2].Add(FaceNormal);
        }
      }
    }
  }

  for (auto& Pair : VertexNormals) {
    int32 VertexIndex = Pair.Key;
    TArray<FVector>& FaceNormals = Pair.Value;

    FVector AverageNormal = FVector::ZeroVector;
    for (const FVector& FaceNormal : FaceNormals) {
      AverageNormal += FaceNormal;
    }

    if (FaceNormals.Num() > 0) {
      AverageNormal = AverageNormal / FaceNormals.Num();
      Normals[VertexIndex] = AverageNormal.GetSafeNormal();
    }
  }

  Mesh->ClearAllMeshSections();
  Mesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs,
                                      Colors, TArray<FProcMeshTangent>(), true);
  Mesh->SetMaterial(0, Material);
}

FVector AVoxelChunk::RoundVector(const FVector& InVector,
                                 float Precision) const {
  return FVector(FMath::RoundToFloat(InVector.X / Precision) * Precision,
                 FMath::RoundToFloat(InVector.Y / Precision) * Precision,
                 FMath::RoundToFloat(InVector.Z / Precision) * Precision);
}

FVector2D AVoxelChunk::CalculateUV(const FVector& Position) const {
  FVector AbsNormal = FVector(FMath::Abs(Position.X), FMath::Abs(Position.Y),
                              FMath::Abs(Position.Z));

  if (AbsNormal.X >= AbsNormal.Y && AbsNormal.X >= AbsNormal.Z) {
    return FVector2D(Position.Y, Position.Z) * 0.05f;
  } else if (AbsNormal.Y >= AbsNormal.X && AbsNormal.Y >= AbsNormal.Z) {
    return FVector2D(Position.X, Position.Z) * 0.05f;
  } else {
    return FVector2D(Position.X, Position.Y) * 0.05f;
  }
}