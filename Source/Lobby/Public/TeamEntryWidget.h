#pragma once

#include "Blueprint/IUserObjectListEntry.h"
#include "CommonUserWidget.h"
#include "CoreMinimal.h"
#include "TeamEntryWidget.generated.h"

class UCommonListView;
class UCommonTextBlock;
class UTeam;

UCLASS()

class LOBBY_API UTeamEntryWidget : public UCommonUserWidget,
                                   public IUserObjectListEntry {
  GENERATED_BODY()

 protected:
  virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
  virtual void NativeOnEntryReleased() override;

  UFUNCTION()
  void HandleTeamMembersChanged() const;

 private:
  UPROPERTY(meta = (BindWidget))
  TObjectPtr<UCommonTextBlock> TeamName;

  UPROPERTY(meta = (BindWidget))
  TObjectPtr<UCommonTextBlock> TeamMemberCount;

  UPROPERTY(meta = (BindWidget))
  TObjectPtr<UCommonListView> PlayerList;

  UPROPERTY()
  TObjectPtr<UTeam> Team;
};
