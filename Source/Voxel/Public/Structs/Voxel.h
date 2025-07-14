#pragma once

#include "CoreMinimal.h"

// clang-format off
#include "Voxel.generated.h"
// clang-format on

USTRUCT()
struct FVoxel {
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere)
  int32 ID;

  UPROPERTY(VisibleAnywhere)
  float Durability;

  FVoxel() : ID(0), Durability(0.f) {}
  FVoxel(const int32 InID, const float InDurability)
      : ID(InID), Durability(InDurability) {}
};
