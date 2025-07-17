#include "VectorPlayerCharacter.h"

#include <Kismet/GameplayStatics.h>

#include "Actors/VoxelWorldActor.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Managers/BuildManager.h"
#include "Managers/DataManager.h"
#include "Managers/DebugManager.h"

AVectorPlayerCharacter::AVectorPlayerCharacter() {
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

  static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(
      TEXT("/Engine/BasicShapes/Sphere.Sphere"));

  if (SphereMeshAsset.Succeeded()) {
    Mesh->SetStaticMesh(SphereMeshAsset.Object);

    constexpr float DefaultSphereMeshRadius = 50.f;

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

  World = Cast<AVoxelWorldActor>(UGameplayStatics::GetActorOfClass(
      GetWorld(), AVoxelWorldActor::StaticClass()));
}

void AVectorPlayerCharacter::BeginPlay() { Super::BeginPlay(); }

void AVectorPlayerCharacter::Tick(const float DeltaTime) {
  Super::Tick(DeltaTime);
}

void AVectorPlayerCharacter::Move(const FInputActionValue &Value) const {
  const FVector MoveValue = Value.Get<FVector>();

  const FVector MoveDirection = GetActorForwardVector() * MoveValue.Y +
                                GetActorRightVector() * MoveValue.X +
                                GetActorUpVector() * MoveValue.Z;

  Collider->AddForce(MoveDirection.GetSafeNormal() * 100, NAME_None, true);
}

void AVectorPlayerCharacter::Look(const FInputActionValue &Value) {
  const FVector2D LookValue = Value.Get<FVector2D>();

  AddActorLocalRotation(FRotator(-LookValue.Y, LookValue.X, 0.f));
}

void AVectorPlayerCharacter::Roll(const FInputActionValue &Value) {
  const float RollValue = Value.Get<float>();

  AddActorLocalRotation(FRotator(0, 0, RollValue));
}

void AVectorPlayerCharacter::Fire() const {
  Collider->AddImpulse(-Camera->GetForwardVector() * 50, NAME_None, true);

  const FVector StartLocation = Camera->GetComponentLocation();
  const FVector EndLocation =
      StartLocation + Camera->GetForwardVector() * 2000.f;

  FHitResult HitResult;
  FCollisionQueryParams Params;
  Params.AddIgnoredActor(this);

  const bool bHit = GetWorld()->LineTraceSingleByChannel(
      HitResult, StartLocation, EndLocation, ECC_Visibility, Params);

  if (bHit) {
    if (Poop) {
      const FIntVector CenterGlobalCoord =
          World->GetDataManager()->WorldToGlobalCoord(HitResult.ImpactPoint);

      World->GetBuildManager()->ConstructVoxelsInRadius(CenterGlobalCoord, 100,
                                                        10, Poop);

      World->GetDebugManager()->SetDebugVoxel(CenterGlobalCoord,
                                              FColor::Yellow);
      World->GetDebugManager()->FlushDebugVoxelBuffer();
    }
  }
}

void AVectorPlayerCharacter::Eat() const {
  const FVector StartLocation = Camera->GetComponentLocation();
  const FVector EndLocation =
      StartLocation + Camera->GetForwardVector() * 2000.f;

  FHitResult HitResult;
  FCollisionQueryParams Params;
  Params.AddIgnoredActor(this);

  const bool bHit = GetWorld()->LineTraceSingleByChannel(
      HitResult, StartLocation, EndLocation, ECC_Visibility, Params);

  if (bHit) {
    const FIntVector CenterGlobalCoord =
        World->GetDataManager()->WorldToGlobalCoord(HitResult.ImpactPoint);
    World->GetBuildManager()->DamageVoxelsInRadius(CenterGlobalCoord, 100, 10);

    World->GetDebugManager()->SetDebugVoxel(CenterGlobalCoord, FColor::Yellow);
    World->GetDebugManager()->FlushDebugVoxelBuffer();
  }
}