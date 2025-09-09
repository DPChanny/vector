#pragma once

#include "Blueprint/IUserObjectListEntry.h"
#include "CommonUserWidget.h"
#include "CoreMinimal.h"
#include "PlayerEntryWidget.generated.h"

class UCommonTextBlock;

UCLASS()

class LOBBY_API UPlayerEntryWidget : public UCommonUserWidget,
                                     public IUserObjectListEntry {
  GENERATED_BODY()

  virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

  UPROPERTY(meta = (BindWidget))
  TObjectPtr<UCommonTextBlock> PlayerName;
};
