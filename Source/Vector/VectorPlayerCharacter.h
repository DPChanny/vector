#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "VectorPlayerCharacter.generated.h"

class UCameraComponent;

UCLASS()
class VECTOR_API AVectorPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AVectorPlayerCharacter();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> MainCamera;

	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
};
