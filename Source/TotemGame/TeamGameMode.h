#pragma once

#include "CoreMinimal.h"
#include "MatchGameMode.h"
#include "TeamGameMode.generated.h"

UCLASS()
class TOTEMGAME_API ATeamGameMode : public AMatchGameMode
{
	GENERATED_BODY()

public:
	ATeamGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual bool CanDamage_Implementation(class AController* Attacker, class AController* Victim) override;
	virtual void Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType) override;
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;

	UFUNCTION(BlueprintCallable, Category = "Team")
	void ChooseTeam(AController* Player, int32 TeamIndex);

	UFUNCTION(BlueprintCallable, Category = "Team")
	int32 GetSmallestTeam();

	UPROPERTY(EditDefaultsOnly, Category = "Team")
	int32 NumTeams;

	UPROPERTY(EditDefaultsOnly, Category = "Team")
	int32 ScoreLimit;

	UPROPERTY(EditDefaultsOnly, Category = "Team")
	TArray<TSubclassOf<AActor>> TeamStartClasses;

protected:
	virtual void CheckGameOver();
	void InitializeTeams();
};
