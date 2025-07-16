#pragma once

#include "CoreMinimal.h"
#include "MeshManager.generated.h"

class UDataManager;

UCLASS()
class VOXEL_API UMeshManager : public UObject {
  GENERATED_BODY()

public:
  void Initialize();

  void SetDirtyChunk(const FIntVector &GlobalCoord);
  void FlushDirtyChunks();

private:
  UPROPERTY()
  TObjectPtr<UDataManager> DataManager;

  UPROPERTY()
  TSet<FIntVector> DirtyChunkCoords;
};
