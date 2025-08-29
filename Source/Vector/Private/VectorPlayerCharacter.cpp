#include "VectorPlayerCharacter.h"

#include "Actors/VoxelWorldActor.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DataAssets/VoxelEntityDataAsset.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Managers/BuildManager.h"
#include "Managers/DataManager.h"
#include "Managers/DebugManager.h"
#include "VectorPlayerState.h"

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
  Light->SetIntensity(5.f);
  Light->SetOuterConeAngle(30.f);
  Light->SetInnerConeAngle(0.f);
  Light->SetAttenuationRadius(200.f);
  Light->SetSourceRadius(25.f);
}

void AVectorPlayerCharacter::BeginPlay() {
  Super::BeginPlay();

  VoxelWorld = UGameplayStatics::GetActorOfClass(
      GetWorld(), AVoxelWorldActor::StaticClass());
  VectorPlayerState = GetPlayerState<AVectorPlayerState>();
}

void AVectorPlayerCharacter::Tick(const float DeltaTime) {
  Super::Tick(DeltaTime);
}

void AVectorPlayerCharacter::Move(const FInputActionValue& Value) const {
  const FVector MoveValue = Value.Get<FVector>();

  const FVector MoveDirection = GetActorForwardVector() * MoveValue.Y +
                                GetActorRightVector() * MoveValue.X +
                                GetActorUpVector() * MoveValue.Z;

  Collider->AddForce(MoveDirection.GetSafeNormal() * CurrentSpeed, NAME_None,
                     true);
}

void AVectorPlayerCharacter::Look(const FInputActionValue& Value) {
  const FVector2D LookValue = Value.Get<FVector2D>();

  AddActorLocalRotation(FRotator(-LookValue.Y, LookValue.X, 0.f));
}

void AVectorPlayerCharacter::Roll(const FInputActionValue& Value) {
  const float RollValue = Value.Get<float>();

  AddActorLocalRotation(FRotator(0, 0, RollValue));
}

void AVectorPlayerCharacter::Fire() const {
  Collider->AddImpulse(-Camera->GetForwardVector() * Recoil, NAME_None, true);

  const FVector StartLocation = Camera->GetComponentLocation();
  const FVector EndLocation =
      StartLocation + Camera->GetForwardVector() * Distance;

  FHitResult HitResult;
  FCollisionQueryParams Params;
  Params.AddIgnoredActor(this);

  const bool bHit = GetWorld()->LineTraceSingleByChannel(
      HitResult, StartLocation, EndLocation, ECC_Visibility, Params);

  if (bHit) {
    if (Poop) {
      const TObjectPtr<UBuildManager> BuildManager =
          VoxelWorld->GetComponentByClass<UBuildManager>();
      const TObjectPtr<UDataManager> DataManager =
          VoxelWorld->GetComponentByClass<UDataManager>();

      if (DataManager) {
        const FIntVector CenterGlobalCoord =
            DataManager->WorldToGlobalCoord(HitResult.ImpactPoint);

        if (VectorPlayerState && BuildManager) {
          BuildManager->ConstructBlocksInRadius(
              CenterGlobalCoord, Range, 10, Poop,
              FVoxelEntityParams(0, VectorPlayerState->TeamName));
        }

        if (const TObjectPtr<UDebugManager> DebugManager =
                VoxelWorld->GetComponentByClass<UDebugManager>()) {
          DebugManager->SetDebugVoxel(CenterGlobalCoord, FColor::Yellow);
          DebugManager->FlushDebugVoxelBuffer();
        }
      }
    }
  }
}

void AVectorPlayerCharacter::OnDamage_Implementation(const FVector HitPoint,
                                                     const float DamageAmount,
                                                     const float DamageRange) {
  IDamageable::OnDamage_Implementation(HitPoint, DamageAmount, DamageRange);

  Health -= DamageAmount;
}

void AVectorPlayerCharacter::PostInitializeComponents() {
  Super::PostInitializeComponents();

  Health = MaxHealth;
  CurrentSpeed = WalkSpeed;
}

void AVectorPlayerCharacter::Eat() const {
  const FVector StartLocation = Camera->GetComponentLocation();
  const FVector EndLocation =
      StartLocation + Camera->GetForwardVector() * Distance;

  FHitResult HitResult;
  FCollisionQueryParams Params;
  Params.AddIgnoredActor(this);

  const bool bHit = GetWorld()->LineTraceSingleByChannel(
      HitResult, StartLocation, EndLocation, ECC_Visibility, Params);

  if (bHit) {
    if (const TObjectPtr<AActor> HitActor = HitResult.GetActor()) {
      if (HitActor->GetClass()->ImplementsInterface(
              UDamageable::StaticClass())) {
        Execute_OnDamage(HitActor, HitResult.ImpactPoint, 10, Range);
      }
    }
  }
}