#pragma once

#include "CoreMinimal.h"
#include "MatchGameMode.h"
#include "FFAGameMode.generated.h"

UCLASS()
class TOTEMGAME_API AFFAGameMode : public AMatchGameMode
{
	GENERATED_BODY()

public:
	AFFAGameMode();

	virtual bool CanDamage_Implementation(class AController* Attacker, class AController* Victim) override;

	virtual void Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType) override;

	UPROPERTY(EditDefaultsOnly, Category = "FFA")
	int32 ScoreLimit;

protected:
	virtual void CheckGameOver();
};
