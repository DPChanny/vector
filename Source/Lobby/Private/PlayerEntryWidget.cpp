#include "PlayerEntryWidget.h"
#include "CommonTextBlock.h"
#include "GameFramework/PlayerState.h"

void UPlayerEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject) {
  IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

  if (const TObjectPtr<APlayerState> PlayerState =
          Cast<APlayerState>(ListItemObject)) {
    PlayerName->SetText(FText::FromString(PlayerState->GetPlayerName()));
  } else {
    PlayerName->SetText(FText::FromString(TEXT("Internal Error")));
  }
}