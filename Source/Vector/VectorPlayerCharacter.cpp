#include "VectorPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "DrawDebugHelpers.h"
#include "VoxelWorld.h"
#include <Kismet/KismetMathLibrary.h>
#include <Kismet/GameplayStatics.h>

AVectorPlayerCharacter::AVectorPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Collider = CreateDefaultSubobject<USphereComponent>(TEXT("Collider"));
	RootComponent = Collider;
	Collider->SetSphereRadius(25.f);
	Collider->SetCollisionProfileName(TEXT("Pawn"));
	Collider->SetEnableGravity(false);
	Collider->SetSimulatePhysics(true);
	Collider->SetLinearDamping(1.f);
	Collider->SetAngularDamping(1.f);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetOwnerNoSee(true);
	Mesh->SetCollisionProfileName(TEXT("NoCollision"));
	Mesh->SetSimulatePhysics(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	if (SphereMeshAsset.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMeshAsset.Object);

		const float DefaultSphereMeshRadius = 50.f;

		const float TargetRadius = Collider->GetUnscaledSphereRadius();
		const float ScaleValue = TargetRadius / DefaultSphereMeshRadius;

		Mesh->SetRelativeScale3D(FVector(ScaleValue));
	}

	Light = CreateDefaultSubobject<USpotLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Camera);

	Light->SetIntensityUnits(ELightUnits::Lumens);
	Light->SetIntensity(50.f);
	Light->SetOuterConeAngle(40.f);
	Light->SetInnerConeAngle(0.f);
	Light->SetAttenuationRadius(500.f);
	Light->SetSourceRadius(25.f);

	World = Cast<AVoxelWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), AVoxelWorld::StaticClass()));
}

void AVectorPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AVectorPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AVectorPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector MoveAxisVector = Value.Get<FVector>();

	const FVector ForceDirection = GetActorForwardVector() * MoveAxisVector.Y + GetActorRightVector() * MoveAxisVector.X + GetActorUpVector() * MoveAxisVector.Z;

	Collider->AddForce(ForceDirection.GetSafeNormal() * 100, NAME_None, true);
}

void AVectorPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddActorLocalRotation(FRotator(-LookAxisVector.Y, LookAxisVector.X, 0.f));
}

void AVectorPlayerCharacter::Roll(const FInputActionValue& Value)
{
	const float value = Value.Get<float>();

	AddActorLocalRotation(FRotator(0, 0, value));
}

void AVectorPlayerCharacter::Fire()
{
	Collider->AddImpulse(-Camera->GetForwardVector() * 50, NAME_None, true);

	FVector StartLocation = Camera->GetComponentLocation();
	FVector EndLocation = StartLocation + Camera->GetForwardVector() * 2000.f;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, Params);

	if (bHit)
		World->ConstructVoxel(HitResult.ImpactPoint, 100, 25, World->GetDefaultBlockID());
}

void AVectorPlayerCharacter::Eat() {
	FVector StartLocation = Camera->GetComponentLocation();
	FVector EndLocation = StartLocation + Camera->GetForwardVector() * 2000.f;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, Params);

	if (bHit)
		World->DamageVoxel(HitResult.ImpactPoint, 100, 25);
}