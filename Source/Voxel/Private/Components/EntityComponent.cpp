#include "Components/EntityComponent.h"

#include "Actors/EntityChunkActor.h"

UEntityComponent::UEntityComponent() {
  bWantsInitializeComponent = true;
}

void UEntityComponent::InitializeComponent() {
  Super::InitializeComponent();

  EntityChunkActor = Cast<AEntityChunkActor>(GetOwner());
}