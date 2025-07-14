#pragma once

#include "CoreMinimal.h"
#include "VoxelMesh.generated.h"

class UVoxelData;

UCLASS()
class VOXEL_API UVoxelMesh : public UObject {
  GENERATED_BODY()

public:
  void Initialize();

  void SetDirtyChunk(const FIntVector &GlobalCoord);
  void FlushDirtyChunks();

private:
  UPROPERTY()
  TObjectPtr<UVoxelData> VoxelData;

  UPROPERTY()
  TSet<FIntVector> DirtyChunkCoords;
};
