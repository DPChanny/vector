#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VectorPlayerCharacter.generated.h"

struct FInputActionValue;
class AVoxelWorldActor;
class USphereComponent;
class UCameraComponent;
class UStaticMeshComponent;
class USpotLightComponent;
class UVoxelBlockDataAsset;

UCLASS()

class PLAYER_API AVectorPlayerCharacter : public APawn {
  GENERATED_BODY()

 public:
  AVectorPlayerCharacter();

  void Move(const FInputActionValue& Value) const;
  void Look(const FInputActionValue& Value);
  void Roll(const FInputActionValue& Value);
  void Eat() const;
  void Fire() const;

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

  UPROPERTY()
  TObjectPtr<AVoxelWorldActor> World;

 private:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;
};