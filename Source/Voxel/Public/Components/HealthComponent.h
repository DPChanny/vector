#pragma once

#include "CoreMinimal.h"
#include "EntityComponent.h"
#include "HealthComponent.generated.h"

UCLASS()

class VOXEL_API UHealthComponent : public UEntityComponent {
  GENERATED_BODY()

 public:
  virtual void InitializeComponent() override;

  virtual void OnEntityAdded(const FIntVector& GlobalCoord,
                             const FVoxelEntityData& NewEntityData) override;
  virtual void OnEntityRemoved(const FIntVector& GlobalCoord) override;
  virtual void OnEntityModified(const FIntVector& GlobalCoord,
                                const FVoxelEntityData& NewEntityData) override;

  UPROPERTY(VisibleAnywhere)
  float MaxHealth;
  UPROPERTY(VisibleAnywhere)
  float CurrentHealth;
};