#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MatchPlayerController.generated.h"

class UUserWidget;
class AMatchPlayerState;

UCLASS()
class TOTEMGAME_API AMatchPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMatchPlayerController();

	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION(BlueprintNativeEvent, Category = "HUD")
	void OnRespawn();

	UFUNCTION(BlueprintNativeEvent, Category = "Match")
	void OnReady();

	UFUNCTION(Client, Reliable)
	void ClientOnGameOver();

	UFUNCTION(Client, Reliable)
	void ClientOnRespawn();

	UFUNCTION(Client, Reliable)
	void ClientOnMatchStart();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void OpenPrimaryHUD();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void OpenPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetPrimaryHUDVisibility(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetCrosshairVisibility(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	virtual void SetModeSpecificHUDVisibility(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void AddToKillfeed(const FString& KillerName, const FString& VictimName, bool bSuicide);

	UFUNCTION(BlueprintCallable, Category = "Match")
	void DestroyAndRespawn();

	UFUNCTION(BlueprintCallable, Category = "Match")
	void StartCameraFade(float FromAlpha, float ToAlpha, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Match")
	void SetTotemPaused(bool bPaused);

	UFUNCTION(BlueprintCallable, Category = "Match")
	void OnGameOver();

	UFUNCTION(BlueprintCallable, Category = "Match")
	void OnEndGame();

	UFUNCTION(BlueprintNativeEvent, Category = "HUD")
	void OnEndCountdown();

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> PrimaryHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> ScoreboardWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> PreMatchWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> RespawningWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> ShopWidgetClass;

protected:
	UPROPERTY(Transient)
	UUserWidget* PrimaryHUDWidget;

	UPROPERTY(Transient)
	UUserWidget* CrosshairWidget;

	UPROPERTY(Transient)
	UUserWidget* ScoreboardWidget;

	UPROPERTY(Transient)
	UUserWidget* PauseMenuWidget;

	UPROPERTY(Transient)
	UUserWidget* PreMatchWidget;

	UPROPERTY(Transient)
	UUserWidget* RespawningWidget;

	UPROPERTY(Transient)
	UUserWidget* ShopWidget;

	UPROPERTY(Transient)
	bool bPrimaryHUDVisible;

	UPROPERTY(Transient)
	bool bCrosshairVisible;

	UPROPERTY(Transient)
	bool bScoreboardVisible;

	UPROPERTY(Transient)
	bool bShopVisible;

	UPROPERTY(Transient)
	bool bModeSpecificHUDVisible;

	UPROPERTY(Transient)
	bool bPaused;

	UPROPERTY(Transient)
	bool bIsInteracting;

	UPROPERTY(Transient)
	bool bIsSpectatorMode;

	UPROPERTY(EditDefaultsOnly, Category = "Respawn")
	float RespawnDelay;
};
