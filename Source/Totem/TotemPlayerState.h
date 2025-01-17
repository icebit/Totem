// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TotemPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class TOTEM_API ATotemPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	void ScoreKill(ATotemPlayerState* Victim);

	void ScoreDeath(ATotemPlayerState* KilledBy);

	UFUNCTION(BlueprintNativeEvent, Category = Score)
	void InformAboutKill(class ATotemPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class ATotemPlayerState* KilledPlayerState);

	UFUNCTION(BlueprintNativeEvent, Category = Score)
	void InformAboutDeath(class ATotemPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class ATotemPlayerState* KilledPlayerState);

	UFUNCTION(BlueprintCallable, Category = Score)
	int32 GetKills();

	UFUNCTION(BlueprintCallable, Category = Score)
	int32 GetDeaths();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

protected:
	// Player's kill count
	UPROPERTY(Replicated)
	int32 Kills;

	// Player's death count
	UPROPERTY(Replicated)
	int32 Deaths;
};
