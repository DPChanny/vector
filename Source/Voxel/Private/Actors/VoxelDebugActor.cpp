#include "Actors/VoxelDebugActor.h"

#include "Actors/VoxelWorld.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "DataAssets/VoxelBaseDataAsset.h"
#include "DataAssets/VoxelBlockDataAsset.h"
#include "Managers/DataManager.h"
#include "VoxelDebugWidget.h"

AVoxelDebugActor::AVoxelDebugActor() {
  PrimaryActorTick.bCanEverTick = false;

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

void AVoxelDebugActor::BeginPlay() {
  Super::BeginPlay();

  if (const TObjectPtr<UUserWidget> UserWidget =
          Widget->GetUserWidgetObject()) {
    DisplayWidget = Cast<UVoxelDebugWidget>(UserWidget);
  }
}

void AVoxelDebugActor::Initialize(const FIntVector &InVoxelCoord,
                                  const FColor &Color) {
  if (const TObjectPtr<AVoxelWorld> VoxelWorld =
          Cast<AVoxelWorld>(GetOwner())) {
    DataManager = VoxelWorld->GetDataManager();
  }

  VoxelCoord = InVoxelCoord;

  Box->SetBoxExtent(FVector(DataManager->GetVoxelSize() / 2 * .85f));

  UpdateActor(Color);
}

void AVoxelDebugActor::UpdateActor(const FColor &Color) const {
  if (!DataManager || !DisplayWidget) {
    return;
  }

  const FVoxelBaseData *VoxelData = DataManager->GetVoxelData(VoxelCoord);
  if (!VoxelData) {
    return;
  }

  FString Text;

  Text += FString::Printf(TEXT("%d %d %d\n"), VoxelCoord.X, VoxelCoord.Y,
                          VoxelCoord.Z);

  Text += FString::Printf(TEXT("Name: %s\n"), *VoxelData->DataAsset->VoxelName);

  Text += FString::Printf(TEXT("Density: %f\n"), VoxelData->GetDensity());

  if (const FVoxelBlockData *VoxelBlockData =
          dynamic_cast<const FVoxelBlockData *>(VoxelData)) {
    Text += FString::Printf(TEXT("Durability: %f / %f\n"),
                            VoxelBlockData->Durability,
                            VoxelBlockData->GetBlockDataAsset()->MaxDurability);
  }

  DisplayWidget->UpdateText(Text);

  Box->ShapeColor = Color;
  Box->MarkRenderStateDirty();
}