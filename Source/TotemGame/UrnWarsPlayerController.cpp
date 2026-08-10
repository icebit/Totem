#include "UrnWarsPlayerController.h"
#include "UrnWarsPlayerState.h"
#include "Blueprint/UserWidget.h"

AUrnWarsPlayerController::AUrnWarsPlayerController()
{
}

bool AUrnWarsPlayerController::ServerReady_Validate() { return true; }
void AUrnWarsPlayerController::ServerReady_Implementation()
{
	AUrnWarsPlayerState* PS = GetPlayerState<AUrnWarsPlayerState>();
	if (PS)
	{
		PS->ServerReady();
	}
}

void AUrnWarsPlayerController::OnReady_Implementation() {}
void AUrnWarsPlayerController::OnReadyClicked_Implementation() {}

void AUrnWarsPlayerController::ClientOnUrnDestroyed_Implementation(int32 TeamIndex)
{
	if (UrnDestroyedWidgetClass)
	{
		UrnDestroyedWidget = CreateWidget<UUserWidget>(this, UrnDestroyedWidgetClass);
		if (UrnDestroyedWidget)
		{
			UrnDestroyedWidget->AddToViewport();
		}
	}
}

void AUrnWarsPlayerController::ClientSetWinner_Implementation(int32 WinningTeam)
{
	if (UrnWarsGameOverWidgetClass)
	{
		UrnWarsGameOverWidget = CreateWidget<UUserWidget>(this, UrnWarsGameOverWidgetClass);
		if (UrnWarsGameOverWidget)
		{
			UrnWarsGameOverWidget->AddToViewport(100);
		}
	}
}

void AUrnWarsPlayerController::SetWinner(int32 WinningTeam)
{
	ClientSetWinner(WinningTeam);
}

void AUrnWarsPlayerController::TeamUrnDeath(int32 DestroyedTeam, AController* Destroyer)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AUrnWarsPlayerController* PC = Cast<AUrnWarsPlayerController>(It->Get());
		if (PC)
		{
			PC->ClientOnUrnDestroyed(DestroyedTeam);
		}
	}
}

bool AUrnWarsPlayerController::GetReady() const
{
	const AUrnWarsPlayerState* PS = GetPlayerState<AUrnWarsPlayerState>();
	return PS ? PS->bIsReady : false;
}

void AUrnWarsPlayerController::SetModeSpecificHUDVisibility(bool bVisible)
{
	Super::SetModeSpecificHUDVisibility(bVisible);

	if (UrnWarsHUD)
	{
		UrnWarsHUD->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}
