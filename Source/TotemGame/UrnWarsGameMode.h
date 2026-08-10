#pragma once

#include "CoreMinimal.h"
#include "TeamGameMode.h"
#include "UrnWarsGameMode.generated.h"

class AUrn;
class AUrnWarsGameState;

UCLASS()
class TOTEMGAME_API AUrnWarsGameMode : public ATeamGameMode
{
	GENERATED_BODY()

public:
	AUrnWarsGameMode();

	virtual void BeginPlay() override;
	virtual void Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType) override;
	virtual bool CanDamage_Implementation(class AController* Attacker, class AController* Victim) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UFUNCTION(BlueprintCallable, Category = "UrnWars")
	void OnUrnDestroyed(int32 DestroyedTeam, AController* Destroyer);

	UFUNCTION(BlueprintCallable, Category = "UrnWars")
	void DestroyAndRespawn(AController* Player);

	UFUNCTION(BlueprintCallable, Category = "UrnWars")
	AUrn* GetUrn(int32 TeamIndex);

	UFUNCTION(BlueprintNativeEvent, Category = "UrnWars")
	void OnGameOver(int32 WinningTeam);

protected:
	virtual void CheckGameOver();
	void NotifyUrnDestroyed(int32 DestroyedTeam, AController* Destroyer);

	UPROPERTY(EditDefaultsOnly, Category = "UrnWars")
	float UrnRespawnDelay;
};
