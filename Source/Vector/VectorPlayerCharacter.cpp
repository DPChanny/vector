#include "VectorPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

AVectorPlayerCharacter::AVectorPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));

	MainCamera->SetupAttachment(GetCapsuleComponent());
	MainCamera->SetRelativeLocation(FVector(0.f, 0.f, 90.f));
	MainCamera->bUsePawnControlRotation = true;
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

	if (!Controller) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}


void AVectorPlayerCharacter::Look(const FInputActionValue& Value)
{
	if (!Controller) return;

	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}