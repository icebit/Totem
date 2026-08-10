#pragma once

#include "CoreMinimal.h"
#include "TeamPlayerController.h"
#include "UrnWarsPlayerController.generated.h"

class AUrn;

UCLASS()
class TOTEMGAME_API AUrnWarsPlayerController : public ATeamPlayerController
{
	GENERATED_BODY()

public:
	AUrnWarsPlayerController();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "UrnWars")
	void ServerReady();

	UFUNCTION(BlueprintNativeEvent, Category = "UrnWars")
	void OnReady();

	UFUNCTION(BlueprintNativeEvent, Category = "UrnWars")
	void OnReadyClicked();

	UFUNCTION(Client, Reliable)
	void ClientOnUrnDestroyed(int32 TeamIndex);

	UFUNCTION(Client, Reliable)
	void ClientSetWinner(int32 WinningTeam);

	UFUNCTION(BlueprintCallable, Category = "UrnWars")
	void SetWinner(int32 WinningTeam);

	UFUNCTION(BlueprintCallable, Category = "UrnWars")
	void TeamUrnDeath(int32 DestroyedTeam, AController* Destroyer);

	UFUNCTION(BlueprintCallable, Category = "UrnWars")
	bool GetReady() const;

	UFUNCTION(BlueprintCallable, Category = "UrnWars")
	void SetModeSpecificHUDVisibility(bool bVisible) override;

	UPROPERTY(EditDefaultsOnly, Category = "UrnWars|HUD")
	TSubclassOf<UUserWidget> UrnWarsHUDClass;

	UPROPERTY(EditDefaultsOnly, Category = "UrnWars|HUD")
	TSubclassOf<UUserWidget> UrnWarsReadyWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UrnWars|HUD")
	TSubclassOf<UUserWidget> UrnDestroyedWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UrnWars|HUD")
	TSubclassOf<UUserWidget> UrnWarsGameOverWidgetClass;

protected:
	UPROPERTY(Transient)
	UUserWidget* UrnWarsHUD;

	UPROPERTY(Transient)
	UUserWidget* UrnWarsReadyWidget;

	UPROPERTY(Transient)
	UUserWidget* UrnDestroyedWidget;

	UPROPERTY(Transient)
	UUserWidget* UrnWarsGameOverWidget;
};
