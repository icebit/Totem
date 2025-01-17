// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DamageEvents.h"
#include "TakeHitInfo.generated.h"

// Replicated information on a hit we've taken
USTRUCT()
struct FTakeHitInfo
{
	GENERATED_USTRUCT_BODY()

	// The amount of damage actually applied
	UPROPERTY()
		float ActualDamage;

	// The damage type we were hit with
	UPROPERTY()
		UClass* DamageTypeClass;

	// Who hit us
	UPROPERTY()
		TWeakObjectPtr<class ATotemPawn> PawnInstigator;

	// Who actually caused the damage
	UPROPERTY()
		TWeakObjectPtr<class AActor> DamageCauser;

	// Specifies which DamageEvent below describes the damage received
	UPROPERTY()
		int32 DamageEventClassID;

	// Rather this was a kill
	UPROPERTY()
		bool bKilled;
private:

	// A rolling counter used to ensure the struct is dirty and will replicate
	UPROPERTY()
		uint8 EnsureReplicationByte;

	// Describes general damage
	UPROPERTY()
		FDamageEvent GeneralDamageEvent;

	// Describes point damage, if that is what was received
	UPROPERTY()
		FPointDamageEvent PointDamageEvent;

	// Describes radial damage, if that is what was received
	UPROPERTY()
		FRadialDamageEvent RadialDamageEvent;

public:
	FTakeHitInfo();

	FDamageEvent& GetDamageEvent();
	void SetDamageEvent(const FDamageEvent& DamageEvent);
	void EnsureReplication();
};