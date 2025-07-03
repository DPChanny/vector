#include "VectorPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "VectorPlayerCharacter.h"

void AVectorPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		Subsystem->AddMappingContext(IMC_VectorPlayer, 0);
}

void AVectorPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent)) {
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AVectorPlayerController::Move);
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AVectorPlayerController::Look);
		EnhancedInputComponent->BindAction(IA_Fire, ETriggerEvent::Triggered, this, &AVectorPlayerController::Fire);
		EnhancedInputComponent->BindAction(IA_Roll, ETriggerEvent::Triggered, this, &AVectorPlayerController::Roll);
	}
}

void AVectorPlayerController::Move(const FInputActionValue& Value)
{
	if (!ControlledCharacter)
		ControlledCharacter = Cast<AVectorPlayerCharacter>(GetPawn());

	if (ControlledCharacter)
		ControlledCharacter->Move(Value);
}

void AVectorPlayerController::Look(const FInputActionValue& Value)
{
	if (!ControlledCharacter)
		ControlledCharacter = Cast<AVectorPlayerCharacter>(GetPawn());

	if (ControlledCharacter)
		ControlledCharacter->Look(Value);
}


void AVectorPlayerController::Fire(const FInputActionValue& Value)
{
	if (!ControlledCharacter)
		ControlledCharacter = Cast<AVectorPlayerCharacter>(GetPawn());

	if (ControlledCharacter)
		ControlledCharacter->Fire();
}


void AVectorPlayerController::Roll(const FInputActionValue& Value)
{
	if (!ControlledCharacter)
		ControlledCharacter = Cast<AVectorPlayerCharacter>(GetPawn());

	if (ControlledCharacter)
		ControlledCharacter->Roll(Value);
}