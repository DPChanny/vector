#pragma once

#include "CoreMinimal.h"
#include "Damageable.h"
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

class PLAYER_API AVectorPlayerCharacter : public APawn, public IDamageable {
  GENERATED_BODY()

 public:
  AVectorPlayerCharacter();

  void Move(const FInputActionValue& Value) const;
  void Look(const FInputActionValue& Value);
  void Roll(const FInputActionValue& Value);
  void Eat() const;
  void Fire() const;

  virtual void OnDamage_Implementation(FVector HitPoint, float DamageAmount,
                                       float DamageRange) override;

 protected:
  virtual void PostInitializeComponents() override;

  UPROPERTY(VisibleAnywhere, Category = "Player | Components")
  TObjectPtr<USphereComponent> Collider;

  UPROPERTY(VisibleAnywhere, Category = "Player | Components")
  TObjectPtr<UCameraComponent> Camera;

  UPROPERTY(VisibleAnywhere, Category = "Player | Components")
  TObjectPtr<UStaticMeshComponent> Mesh;

  UPROPERTY(VisibleAnywhere, Category = "Player | Components")
  TObjectPtr<USpotLightComponent> Light;

  UPROPERTY(EditDefaultsOnly, Category = "Player | Poops")
  TObjectPtr<UVoxelBlockDataAsset> Poop;

  UPROPERTY(VisibleAnywhere, Category = "Player")
  TObjectPtr<AVoxelWorldActor> World;

  UPROPERTY(EditDefaultsOnly, Category = "Player | Stats")
  float WalkSpeed = 100.f;

  UPROPERTY(EditDefaultsOnly, Category = "Player | Stats")
  float RunSpeed = 500.f;

  float CurrentSpeed;

  UPROPERTY(EditDefaultsOnly, Category = "Player | Stats")
  float MaxHealth = 100.f;

  UPROPERTY(EditDefaultsOnly, Category = "Player | Stats")
  float Range = 100.f;

  UPROPERTY(EditDefaultsOnly, Category = "Player | Stats")
  float Distance = 500.f;

  UPROPERTY(EditDefaultsOnly, Category = "Player | Stats")
  float Recoil = 0.f;

  UPROPERTY(VisibleAnywhere, Category = "Player | Stats")
  float Health;

 private:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;
};