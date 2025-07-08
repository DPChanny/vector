#include "VoxelDebugActor.h"

#include <Kismet/GameplayStatics.h>

#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "VoxelBaseDataAsset.h"
#include "VoxelBlockDataAsset.h"
#include "VoxelDebugWidget.h"
#include "VoxelWorld.h"

AVoxelDebugActor::AVoxelDebugActor() {
  PrimaryActorTick.bCanEverTick = true;

  Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
  RootComponent = Box;
  Box->SetHiddenInGame(false);
  Box->SetLineThickness(.5f);
  Box->SetCollisionProfileName(TEXT("NoCollision"));

  Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
  Widget->SetupAttachment(RootComponent);
  Widget->SetWidgetSpace(EWidgetSpace::Screen);
  Widget->SetDrawSize(FVector2D(300.f, 200.f));
}

void AVoxelDebugActor::Initialize(const FIntVector& InVoxelCoord) {
  World = Cast<AVoxelWorld>(UGameplayStatics::GetActorOfClass(
      GetWorld(), AVoxelWorld::StaticClass()));

  VoxelCoord = InVoxelCoord;

  Box->SetBoxExtent(FVector(World->GetVoxelSize() / 2));

  UpdateWidget();
}

void AVoxelDebugActor::BeginPlay() {
  Super::BeginPlay();

  if (UUserWidget* UserWidget = Widget->GetUserWidgetObject()) {
    DisplayWidget = Cast<UVoxelDebugWidget>(UserWidget);
  }
}

void AVoxelDebugActor::UpdateWidget() {
  if (!World.IsValid() || !DisplayWidget) {
    if (UUserWidget* UserWidget = Widget->GetUserWidgetObject()) {
      DisplayWidget = Cast<UVoxelDebugWidget>(UserWidget);
    }
    if (!DisplayWidget) return;
  }

  const int32 VoxelID = World->GetVoxelID(VoxelCoord);
  const float CurrentDurability = World->GetDurability(VoxelCoord);
  const float CurrentDensity = World->GetDensity(VoxelCoord);
  float MaxDurability = 0.f;
  float BaseDensity = 0.f;

  if (const UVoxelBaseDataAsset* VoxelData = World->GetVoxelData(VoxelID)) {
    BaseDensity = VoxelData->BaseDensity;
    if (const UVoxelBlockDataAsset* BlockData =
            Cast<UVoxelBlockDataAsset>(VoxelData)) {
      MaxDurability = BlockData->MaxDurability;
    }
  }

  DisplayWidget->UpdateInfo(VoxelCoord, VoxelID, CurrentDurability,
                            MaxDurability, CurrentDensity, BaseDensity);
}