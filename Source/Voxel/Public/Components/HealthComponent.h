#pragma once

#include "CoreMinimal.h"
#include "EntityComponent.h"
#include "HealthComponent.generated.h"

UCLASS()

class VOXEL_API UHealthComponent : public UEntityComponent {
  GENERATED_BODY()

 public:
  virtual void OnEntityAdded(const FIntVector& VoxelCoord,
                             const FVoxelEntityData& Data) override;
  virtual void OnEntityRemoved(const FIntVector& VoxelCoord,
                               const FVoxelEntityData& Data) override;

  UPROPERTY(VisibleAnywhere)
  float TotalHealth;
  UPROPERTY(VisibleAnywhere)
  float CurrentHealth;
};