// Fill out your copyright notice in the Description page of Project Settings.


#include "TotemPlayerState.h"
#include <Runtime\Engine\Public\Net\UnrealNetwork.h>

void ATotemPlayerState::ScoreKill(ATotemPlayerState* Victim)
{
	Kills++;
}

void ATotemPlayerState::ScoreDeath(ATotemPlayerState* KilledBy)
{
	Deaths++; 
}

void ATotemPlayerState::InformAboutKill_Implementation(ATotemPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, ATotemPlayerState* KilledPlayerState)
{ 
}

void ATotemPlayerState::InformAboutDeath_Implementation(ATotemPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, ATotemPlayerState* KilledPlayerState)
{
}

int32 ATotemPlayerState::GetKills()
{
	return Kills;
}

int32 ATotemPlayerState::GetDeaths()
{
	return Deaths;
}

void ATotemPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATotemPlayerState, Kills);
	DOREPLIFETIME(ATotemPlayerState, Deaths);
}