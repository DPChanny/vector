#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "VectorPlayerCharacter.generated.h"

class AVoxelWorld;
class USphereComponent;
class UCameraComponent;
class UStaticMeshComponent;
class USpotLightComponent;
class UVoxelBlockDataAsset;

UCLASS()
class VECTOR_API AVectorPlayerCharacter : public APawn {
  GENERATED_BODY()

 public:
  AVectorPlayerCharacter();

  void Move(const FInputActionValue& Value);
  void Look(const FInputActionValue& Value);
  void Roll(const FInputActionValue& Value);
  void Eat();
  void Fire();

 protected:
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<USphereComponent> Collider;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<UCameraComponent> Camera;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<UStaticMeshComponent> Mesh;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<USpotLightComponent> Light;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Poops")
  TObjectPtr<UVoxelBlockDataAsset> Poop;

  TObjectPtr<AVoxelWorld> World;

 private:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;
};