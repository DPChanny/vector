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

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AVectorPlayerController::Move);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AVectorPlayerController::Look);
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
