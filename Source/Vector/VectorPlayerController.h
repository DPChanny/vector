#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "VectorPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class AVectorPlayerCharacter;

UCLASS()
class VECTOR_API AVectorPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	TObjectPtr<UInputMappingContext> IMC_VectorPlayer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	TObjectPtr<UInputAction> IA_Fire;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	TObjectPtr<UInputAction> IA_Eat;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	TObjectPtr<UInputAction> IA_Roll;

private:
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

	void Move(const FInputActionValue& Value);
	void Fire(const FInputActionValue& Value);
	void Roll(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Eat(const FInputActionValue& Value);

	UPROPERTY()
	TObjectPtr<AVectorPlayerCharacter> ControlledCharacter;
};
