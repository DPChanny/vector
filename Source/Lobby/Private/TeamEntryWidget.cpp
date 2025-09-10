#include "TeamEntryWidget.h"
#include "CommonListView.h"
#include "CommonTextBlock.h"
#include "LobbyGameState.h"
#include "Team.h"
#include "VectorPlayerState.h"

void UTeamEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject) {
  IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

  Team = Cast<UTeam>(ListItemObject);
  if (!Team) {
    return;
  }

  HandleTeamMembersChanged();
  Team->OnTeamMembersChanged.AddUObject(
      this, &UTeamEntryWidget::HandleTeamMembersChanged);
}

void UTeamEntryWidget::NativeOnEntryReleased() {
  IUserObjectListEntry::NativeOnEntryReleased();

  if (Team) {
    Team->OnTeamMembersChanged.RemoveAll(this);
  }
}

void UTeamEntryWidget::HandleTeamMembersChanged() const {
  if (!Team || !Team->OwningGameState) {
    return;
  }

  TeamName->SetText(FText::FromString(Team->Name));
  const FString MemberCountText =
      FString::Printf(TEXT("(%d / %d)"), Team->Members.Num(),
                      Team->OwningGameState->MaxPlayersPerTeam);
  TeamMemberCount->SetText(FText::FromString(MemberCountText));

  PlayerList->ClearListItems();
  for (const TObjectPtr PlayerState : Team->Members) {
    PlayerList->AddItem(PlayerState);
  }
}
