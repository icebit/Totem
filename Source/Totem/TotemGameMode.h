// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TotemGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TOTEM_API ATotemGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	void Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool CanDamage(class AController* Attacker, class AController* Victim);

	UFUNCTION(BlueprintNativeEvent)
	bool OnTotemKilled(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType);
};
