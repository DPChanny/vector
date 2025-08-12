#pragma once

#include "CoreMinimal.h"
#include "MeshManager.generated.h"

class UDataManager;

UCLASS()

class VOXEL_API UMeshManager : public UActorComponent {
  GENERATED_BODY()

 public:
  virtual void BeginPlay() override;

  void SetDirtyChunk(const FIntVector& GlobalCoord);
  void FlushDirtyChunks();

 private:
  UPROPERTY(VisibleAnywhere)
  TObjectPtr<UDataManager> DataManager;

  TSet<FIntVector> DirtyChunkCoords;
};
