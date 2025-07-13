#include "VoxelDebugActor.h"

#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "VoxelBaseDataAsset.h"
#include "VoxelBlockDataAsset.h"
#include "VoxelData.h"
#include "VoxelDebugWidget.h"

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

void AVoxelDebugActor::Initialize(const FIntVector &InVoxelCoord,
                                  UVoxelData *InVoxelData) {
  VoxelData = InVoxelData;

  VoxelCoord = InVoxelCoord;

  Box->SetBoxExtent(FVector(VoxelData->GetVoxelSize() / 2));

  UpdateWidget();
}

void AVoxelDebugActor::BeginPlay() {
  Super::BeginPlay();

  if (UUserWidget *UserWidget = Widget->GetUserWidgetObject()) {
    DisplayWidget = Cast<UVoxelDebugWidget>(UserWidget);
  }
}

void AVoxelDebugActor::UpdateWidget() {
  if (!DisplayWidget) {
    if (UUserWidget *UserWidget = Widget->GetUserWidgetObject()) {
      DisplayWidget = Cast<UVoxelDebugWidget>(UserWidget);
    }
    if (!DisplayWidget) {
      return;
    }
  }

  const int32 VoxelID = VoxelData->GetVoxelID(VoxelCoord);
  const float CurrentDurability = VoxelData->GetDurability(VoxelCoord);
  const float CurrentDensity = VoxelData->GetDensity(VoxelCoord);
  float MaxDurability = 0.f;
  float BaseDensity = 0.f;

  if (const UVoxelBaseDataAsset *VoxelDataAsset =
          VoxelData->GetVoxelDataAsset(VoxelID)) {
    BaseDensity = VoxelDataAsset->BaseDensity;
    if (const UVoxelBlockDataAsset *BlockData =
            Cast<UVoxelBlockDataAsset>(VoxelDataAsset)) {
      MaxDurability = BlockData->MaxDurability;
    }
  }

  DisplayWidget->UpdateInfo(VoxelCoord, VoxelID, CurrentDurability,
                            MaxDurability, CurrentDensity, BaseDensity);
}