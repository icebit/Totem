#include "MatchPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "MatchPlayerState.h"
#include "TotemPawn.h"

AMatchPlayerController::AMatchPlayerController()
{
	bPrimaryHUDVisible = false;
	bCrosshairVisible = false;
	bScoreboardVisible = false;
	bShopVisible = false;
	bModeSpecificHUDVisible = false;
	bPaused = false;
	bIsInteracting = false;
	bIsSpectatorMode = false;
	RespawnDelay = 3.0f;
}

void AMatchPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IsLocalController())
	{
		OpenPrimaryHUD();
	}
}

void AMatchPlayerController::OnRespawn_Implementation() {}

void AMatchPlayerController::OnReady_Implementation() {}

void AMatchPlayerController::ClientOnGameOver_Implementation()
{
	OnGameOver();
}

void AMatchPlayerController::ClientOnRespawn_Implementation()
{
	if (RespawningWidgetClass)
	{
		RespawningWidget = CreateWidget<UUserWidget>(this, RespawningWidgetClass);
		if (RespawningWidget)
		{
			RespawningWidget->AddToViewport();
		}
	}

	StartCameraFade(0.f, 1.f, RespawnDelay);
}

void AMatchPlayerController::ClientOnMatchStart_Implementation()
{
	if (PreMatchWidget)
	{
		PreMatchWidget->RemoveFromParent();
		PreMatchWidget = nullptr;
	}
}

void AMatchPlayerController::OpenPrimaryHUD()
{
	if (!PrimaryHUDWidget && PrimaryHUDWidgetClass)
	{
		PrimaryHUDWidget = CreateWidget<UUserWidget>(this, PrimaryHUDWidgetClass);
		if (PrimaryHUDWidget)
		{
			PrimaryHUDWidget->AddToViewport();
			bPrimaryHUDVisible = true;
		}
	}

	if (!CrosshairWidget && CrosshairWidgetClass)
	{
		CrosshairWidget = CreateWidget<UUserWidget>(this, CrosshairWidgetClass);
		if (CrosshairWidget)
		{
			CrosshairWidget->AddToViewport();
			bCrosshairVisible = true;
		}
	}
}

void AMatchPlayerController::OpenPauseMenu()
{
	if (!PauseMenuWidget && PauseMenuWidgetClass)
	{
		PauseMenuWidget = CreateWidget<UUserWidget>(this, PauseMenuWidgetClass);
		if (PauseMenuWidget)
		{
			PauseMenuWidget->AddToViewport(100);
			SetInputMode(FInputModeUIOnly());
			bShowMouseCursor = true;
		}
	}
}

void AMatchPlayerController::SetPrimaryHUDVisibility(bool bVisible)
{
	bPrimaryHUDVisible = bVisible;
	if (PrimaryHUDWidget)
	{
		PrimaryHUDWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void AMatchPlayerController::SetCrosshairVisibility(bool bVisible)
{
	bCrosshairVisible = bVisible;
	if (CrosshairWidget)
	{
		CrosshairWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void AMatchPlayerController::SetModeSpecificHUDVisibility(bool bVisible)
{
	bModeSpecificHUDVisible = bVisible;
}

void AMatchPlayerController::AddToKillfeed(const FString& KillerName, const FString& VictimName, bool bSuicide)
{
}

void AMatchPlayerController::DestroyAndRespawn()
{
	APawn* MyPawn = GetPawn();
	if (MyPawn)
	{
		MyPawn->Destroy();
	}
}

void AMatchPlayerController::StartCameraFade(float FromAlpha, float ToAlpha, float Duration)
{
	APlayerCameraManager* CameraManager = PlayerCameraManager;
	if (CameraManager)
	{
		CameraManager->StartCameraFade(FromAlpha, ToAlpha, Duration, FLinearColor::Black);
	}
}

void AMatchPlayerController::SetTotemPaused(bool bPaused)
{
	bPaused = bPaused;
}

void AMatchPlayerController::OnGameOver()
{
	if (PrimaryHUDWidget)
	{
		PrimaryHUDWidget->RemoveFromParent();
		PrimaryHUDWidget = nullptr;
	}
	if (CrosshairWidget)
	{
		CrosshairWidget->RemoveFromParent();
		CrosshairWidget = nullptr;
	}
}

void AMatchPlayerController::OnEndGame()
{
	OnGameOver();
}

void AMatchPlayerController::OnEndCountdown_Implementation() {}
