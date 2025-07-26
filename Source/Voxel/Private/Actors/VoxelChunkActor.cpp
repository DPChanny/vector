#include "Actors/VoxelChunkActor.h"

#include "Actors/VoxelWorldActor.h"
#include "DataAssets/VoxelSubstanceDataAsset.h"
#include "Engine/World.h"
#include "Managers/DataManager.h"
#include "MarchingCubesTables.h"
#include "ProceduralMeshComponent.h"

AVoxelChunkActor::AVoxelChunkActor() : ChunkCoord(FIntVector::ZeroValue) {
  PrimaryActorTick.bCanEverTick = false;

  Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
  RootComponent = Mesh;

  Mesh->bUseAsyncCooking = true;
  Mesh->SetCastShadow(true);
  Mesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void AVoxelChunkActor::Initialize(const FIntVector &InChunkCoord) {
  if (const TObjectPtr<AVoxelWorldActor> VoxelWorld =
          Cast<AVoxelWorldActor>(GetOwner())) {
    DataManager = VoxelWorld->GetDataManager();
  }
  ChunkCoord = InChunkCoord;
}

FVector AVoxelChunkActor::InterpolateVertex(const FVector &P1,
                                            const FVector &P2, const float Val1,
                                            const float Val2) {
  if (FMath::IsNearlyEqual(Val1, Val2)) {
    return P1;
  }
  const float Mu = (UDataManager::GetSurfaceLevel() - Val1) / (Val2 - Val1);
  return P1 + Mu * (P2 - P1);
}

void AVoxelChunkActor::UpdateMesh() const {
  if (!DataManager) {
    return;
  }

  TArray<FVector> Vertices;
  TArray<int32> Triangles;
  TArray<FVector> Normals;
  TArray<FLinearColor> Colors;
  TArray<FVector2D> UVs;

  TMap<FVector, int32> VertexMap;
  TMap<int32, TArray<FVector>> VertexNormals;

  for (int32 x = 0; x < DataManager->GetChunkSize(); ++x) {
    for (int32 y = 0; y < DataManager->GetChunkSize(); ++y) {
      for (int32 z = 0; z < DataManager->GetChunkSize(); ++z) {
        const FIntVector GlobalCoord =
            DataManager->LocalToGlobalCoord(FIntVector(x, y, z), ChunkCoord);

        float CornerDensities[8]{};
        FVector CornerPositions[8];
        int32 CubeIndex = 0;
        for (int i = 0; i < 8; ++i) {
          const FIntVector CornerGlobalCoord = GlobalCoord + CornerOffsets[i];

          if (const FVoxelBaseData *VoxelData =
                  DataManager->GetVoxelData(CornerGlobalCoord)) {
            CornerDensities[i] = VoxelData->GetDensity();
          } else {
            CornerDensities[i] = FVoxelVoidData().GetDensity();
          }
          CornerPositions[i] =
              DataManager->GlobalToWorldCoord(CornerGlobalCoord) -
              GetActorLocation();

          if (CornerDensities[i] > UDataManager::GetSurfaceLevel()) {
            CubeIndex |= (1 << i);
          }
        }

        if (EdgeTable[CubeIndex] == 0) {
          continue;
        }

        FVector EdgeVertices[12];
        FLinearColor EdgeVertexColors[12];

        for (int i = 0; i < 12; ++i) {
          if ((EdgeTable[CubeIndex] & (1 << i))) {
            const int32 CornerA = EdgeConnections[i][0];
            const int32 CornerB = EdgeConnections[i][1];
            EdgeVertices[i] = InterpolateVertex(
                CornerPositions[CornerA], CornerPositions[CornerB],
                CornerDensities[CornerA], CornerDensities[CornerB]);
            if (const FVoxelBaseData *VoxelData = DataManager->GetVoxelData(
                    GlobalCoord +
                    (CornerDensities[CornerA] > CornerDensities[CornerB]
                         ? CornerOffsets[CornerA]
                         : CornerOffsets[CornerB]))) {
              if (const FVoxelSubstanceData *VoxelSubstanceData =
                      dynamic_cast<const FVoxelSubstanceData *>(VoxelData)) {
                if (VoxelSubstanceData->GetSubstanceDataAsset()) {
                  EdgeVertexColors[i] =
                      VoxelSubstanceData->GetSubstanceDataAsset()->VertexColor;
                }
              }
            }
          }
        }

        for (int i = 0; TriTable[CubeIndex][i] != -1; i += 3) {
          const FVector TriangleVertices[3] = {
              EdgeVertices[TriTable[CubeIndex][i]],
              EdgeVertices[TriTable[CubeIndex][i + 1]],
              EdgeVertices[TriTable[CubeIndex][i + 2]]};

          const FVector FaceNormal =
              -FVector::CrossProduct(TriangleVertices[1] - TriangleVertices[0],
                                     TriangleVertices[2] - TriangleVertices[0])
                   .GetSafeNormal();

          for (int j = 0; j < 3; ++j) {
            const FVector RoundedVertex =
                RoundVector(TriangleVertices[j], 0.001f);
            int32 VertexIndex;

            if (int32 *ExistingIndex = VertexMap.Find(RoundedVertex)) {
              VertexIndex = *ExistingIndex;
            } else {
              VertexIndex = Vertices.Add(TriangleVertices[j]);
              Normals.Add(FVector::ZeroVector);
              Colors.Add(EdgeVertexColors[TriTable[CubeIndex][i + j]]);
              UVs.Add(GetUV(TriangleVertices[j]));
              VertexMap.Add(RoundedVertex, VertexIndex);
              VertexNormals.Add(VertexIndex, TArray<FVector>());
            }

            Triangles.Add(VertexIndex);
            VertexNormals[VertexIndex].Add(FaceNormal);
          }
        }
      }
    }
  }

  for (auto &Pair : VertexNormals) {
    int32 VertexIndex = Pair.Key;
    TArray<FVector> &FaceNormals = Pair.Value;

    FVector AverageNormal = FVector::ZeroVector;
    for (const FVector &FaceNormal : FaceNormals) {
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

FVector AVoxelChunkActor::RoundVector(const FVector &InVector,
                                      const float Precision) {
  return FVector(FMath::RoundToFloat(InVector.X / Precision) * Precision,
                 FMath::RoundToFloat(InVector.Y / Precision) * Precision,
                 FMath::RoundToFloat(InVector.Z / Precision) * Precision);
}

FVector2D AVoxelChunkActor::GetUV(const FVector &Position) {
  const FVector AbsNormal = FVector(
      FMath::Abs(Position.X), FMath::Abs(Position.Y), FMath::Abs(Position.Z));

  if (AbsNormal.X >= AbsNormal.Y && AbsNormal.X >= AbsNormal.Z) {
    return FVector2D(Position.Y, Position.Z) * 0.05f;
  }
  if (AbsNormal.Y >= AbsNormal.X && AbsNormal.Y >= AbsNormal.Z) {
    return FVector2D(Position.X, Position.Z) * 0.05f;
  }
  return FVector2D(Position.X, Position.Y) * 0.05f;
}