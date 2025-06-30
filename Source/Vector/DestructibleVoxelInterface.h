#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DestructibleVoxelInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UDestructibleVoxelInterface : public UInterface
{
	GENERATED_BODY()
};

class IDestructibleVoxelInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Voxel|Durability")
	float GetDurability() const;

	UFUNCTION(Server, Reliable, WithValidation)
	virtual void Server_OnDamage(float DamageAmount, AActor* DamageCauser) = 0;
};