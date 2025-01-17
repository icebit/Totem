// Fill out your copyright notice in the Description page of Project Settings.


#include "TotemGameMode.h"
#include "GameFramework/PlayerController.h"
#include "TotemPlayerState.h"

void ATotemGameMode::Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	ATotemPlayerState* KillerPlayerState = Killer ? Cast<ATotemPlayerState>(Killer->PlayerState) : NULL;
	ATotemPlayerState* VictimPlayerState = KilledPlayer ? Cast<ATotemPlayerState>(KilledPlayer->PlayerState) : NULL;

	if (KillerPlayerState && KillerPlayerState != VictimPlayerState)
	{
		KillerPlayerState->ScoreKill(VictimPlayerState);
		KillerPlayerState->InformAboutKill(KillerPlayerState, DamageType, VictimPlayerState);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->ScoreDeath(KillerPlayerState);
		VictimPlayerState->InformAboutDeath(KillerPlayerState, DamageType, VictimPlayerState);
	}

	Killer = Killer ? Killer : NULL;
	KilledPlayer = KilledPlayer ? KilledPlayer : NULL;

	OnTotemKilled(Killer, KilledPlayer, KilledPawn, DamageType);
}

bool ATotemGameMode::CanDamage_Implementation(class AController* Attacker, class AController* Victim)
{
	return true;
}

bool ATotemGameMode::OnTotemKilled_Implementation(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	return false;
}
