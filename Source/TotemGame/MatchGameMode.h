#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TotemGameMode.h"
#include "MatchGameMode.generated.h"

class AMatchGameState;
class AMatchPlayerState;

UENUM(BlueprintType)
enum class EMatchState : uint8
{
	PendingStart,
	Countdown,
	InProgress,
	GameOver
};

UCLASS()
class TOTEMGAME_API AMatchGameMode : public ATotemGameMode
{
	GENERATED_BODY()

public:
	AMatchGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType) override;

	UFUNCTION(BlueprintCallable, Category = "Match")
	void CheckReady();

	UFUNCTION(BlueprintCallable, Category = "Match")
	void BeginCountdown();

	UFUNCTION(BlueprintCallable, Category = "Match")
	void BeginMatch();

	UFUNCTION(BlueprintCallable, Category = "Match")
	void EndMatch();

	UFUNCTION(BlueprintNativeEvent, Category = "Match")
	void OnStartMatch();

	UFUNCTION(BlueprintNativeEvent, Category = "Match")
	void OnEndCountdown();

	UFUNCTION(BlueprintNativeEvent, Category = "Match")
	void NotifyTotemKilled(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType);

	UFUNCTION(BlueprintCallable, Category = "Match")
	int32 GetNumReadyPlayers();

	UFUNCTION(BlueprintCallable, Category = "Match")
	bool AreAllPlayersReady();

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	int32 RequiredNumPlayers;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	float CountdownDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	EMatchState MatchState;

	UPROPERTY(EditDefaultsOnly, Category = "Spectator")
	TSubclassOf<AActor> SpectatorStartClass;

protected:
	FTimerHandle CountdownTimer;

	void SetMatchState(EMatchState NewState);

	bool bAllReady;
};
