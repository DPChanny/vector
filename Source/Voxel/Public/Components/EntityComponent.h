#pragma once

#include "CoreMinimal.h"
#include "EntityComponent.generated.h"

class AEntityChunkActor;
struct FVoxelBaseData;
struct FVoxelEntityData;

UCLASS(Abstract, meta = (BlueprintSpawnableComponent), Blueprintable)

class VOXEL_API UEntityComponent : public UActorComponent {
  GENERATED_BODY()

 public:
  virtual void OnEntityAdded(const FIntVector& GlobalCoord,
                             const FVoxelEntityData& NewEntityData) {}

  virtual void OnEntityRemoved(const FIntVector& GlobalCoord) {}

  virtual void OnEntityModified(const FIntVector& GlobalCoord,
                                const FVoxelEntityData& NewEntityData) {}

 protected:
  UEntityComponent();

  virtual void InitializeComponent() override;

  UPROPERTY()
  TObjectPtr<AEntityChunkActor> EntityChunkActor;
};
