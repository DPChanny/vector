#include "EntityComponent.h"

#include "EntityChunk.h"

UEntityComponent::UEntityComponent() {
  OwnerChunk = Cast<UEntityChunk>(GetOuter());
}