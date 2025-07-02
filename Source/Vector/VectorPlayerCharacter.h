#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "InputActionValue.h"
#include "VectorPlayerCharacter.generated.h"


UCLASS()
class VECTOR_API AVectorPlayerCharacter : public APawn
{
	GENERATED_BODY()

public:
	AVectorPlayerCharacter();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* Collider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpotLightComponent* Light;

	bool bIsGrounded;
	FVector GroundNormal;

	float CameraPitch;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float MinCameraPitch = -89.f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float MaxCameraPitch = 89.f;

	UFUNCTION()
	void OnColliderHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

public:
	virtual void Tick(float DeltaTime) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Fire();
	void SetIsGrounded(bool bNewGroundedState, FVector NewGroundNormal);
};