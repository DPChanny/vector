#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "InputActionValue.h"
#include "VectorPlayerCharacter.generated.h"


class AVoxelWorld;

UCLASS()
class VECTOR_API AVectorPlayerCharacter : public APawn
{
	GENERATED_BODY()

public:
	AVectorPlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr < USphereComponent> Collider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr < UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr < UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpotLightComponent> Light;

	TObjectPtr<AVoxelWorld> World;

public:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Roll(const FInputActionValue& Value);
	void Fire();
};