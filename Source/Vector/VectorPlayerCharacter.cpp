#include "VectorPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "DrawDebugHelpers.h"
#include <Kismet/KismetMathLibrary.h>

AVectorPlayerCharacter::AVectorPlayerCharacter()
{
	bIsGrounded = false;
	CameraPitch = 0.f;
	GroundNormal = FVector::UpVector;

	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Collider = CreateDefaultSubobject<USphereComponent>(TEXT("Collider"));
	Collider->SetupAttachment(Root);
	Collider->SetSphereRadius(50.0f);
	Collider->SetCollisionProfileName(TEXT("VectorPlayerCharacter"));
	Collider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Collider->SetEnableGravity(false);
	Collider->SetSimulatePhysics(true);
	Collider->OnComponentHit.AddDynamic(this, &AVectorPlayerCharacter::OnColliderHit);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	Mesh->SetOwnerNoSee(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetSimulatePhysics(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	if (SphereMeshAsset.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMeshAsset.Object);
	}

	Light = CreateDefaultSubobject<USpotLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Camera);

	Light->SetIntensityUnits(ELightUnits::Lumens);
	Light->SetIntensity(100.f);

	Light->SetOuterConeAngle(30.f);
	Light->SetInnerConeAngle(0.f);

	Light->SetAttenuationRadius(500.f);
	Light->SetSourceRadius(50.f);
}

void AVectorPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AVectorPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsGrounded)
		if (Collider)
			SetActorLocation(Collider->GetComponentLocation());

	if (Collider && Mesh)
		Mesh->SetWorldRotation(Collider->GetComponentRotation());
}

void AVectorPlayerCharacter::Move(const FInputActionValue& Value)
{

}

void AVectorPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (bIsGrounded)
	{
		AddActorLocalRotation(FRotator(0.f, LookAxisVector.X, 0.f));
		CameraPitch = FMath::Clamp(CameraPitch - LookAxisVector.Y, MinCameraPitch, MaxCameraPitch);
		Camera->SetRelativeRotation(FRotator(CameraPitch, 0.f, 0.f));
	}
	else
	{
		AddActorLocalRotation(FRotator(-LookAxisVector.Y, LookAxisVector.X, 0.f));
	}
}

void AVectorPlayerCharacter::Fire()
{
	Collider->AddImpulse(-Camera->GetForwardVector() * 10, NAME_None, true);

	//FVector StartLocation = MainCamera->GetComponentLocation();
	//FVector EndLocation = StartLocation + MainCamera->GetForwardVector() * 2000.f;

	//FHitResult HitResult;
	//FCollisionQueryParams Params;
	//Params.AddIgnoredActor(this);

	//bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, Params);

	//DrawDebugLine(GetWorld(), StartLocation, EndLocation, bHit ? FColor::Green : FColor::Red, false, 2.f);

	//if (bHit)
	//{
	//	if (GEngine)
	//	{
	//		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("Hit: %s"), *HitResult.GetActor()->GetName()));
	//	}
	//}
}

void AVectorPlayerCharacter::SetIsGrounded(bool bNewGroundedState, FVector NewGroundNormal)
{
	if (bIsGrounded == bNewGroundedState) return;

	bIsGrounded = bNewGroundedState;
	GroundNormal = NewGroundNormal.GetSafeNormal();

	Collider->SetSimulatePhysics(!bIsGrounded);

	if (bIsGrounded)
	{
		const FVector OriginalCameraForward = Camera->GetForwardVector();

		const FVector ProjectedActorForward = FVector::VectorPlaneProject(GetActorForwardVector(), GroundNormal).GetSafeNormal();
		const FRotator TargetActorRotation = UKismetMathLibrary::MakeRotFromXZ(ProjectedActorForward, GroundNormal);
		SetActorRotation(TargetActorRotation);

		const FRotator TargetCameraRotation = UKismetMathLibrary::MakeRotFromXZ(OriginalCameraForward, GroundNormal);
		Camera->SetWorldRotation(TargetCameraRotation);

		CameraPitch = Camera->GetRelativeRotation().Pitch;

	}
	else
	{
		const FRotator CurrentRotation = Camera->GetComponentRotation();
		Camera->SetRelativeRotation(FRotator::ZeroRotator);
		SetActorRotation(CurrentRotation);
		Collider->SetWorldLocation(GetActorLocation());
	}
}

void AVectorPlayerCharacter::OnColliderHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 2.f, FColor::Green,
			FString::Printf(TEXT("HI"))
		);
	}

	if (bIsGrounded)
	{
		return;
	}

	if (OtherActor == nullptr || OtherActor == this)
	{
		return;
	}

	const FVector MyVelocity = Collider->GetPhysicsLinearVelocity();
	if (FVector::DotProduct(MyVelocity, Hit.ImpactNormal) >= 0.f)
	{
		return;
	}


	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 2.f, FColor::Green,
			FString::Printf(TEXT("Sticking to surface! Normal: %s"), *Hit.ImpactNormal.ToString())
		);
	}

	SetIsGrounded(true, Hit.ImpactNormal);
}