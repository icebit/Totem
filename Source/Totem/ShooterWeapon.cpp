// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterWeapon.h"
#include "BaseProjectile.h"
#include "Camera/CameraComponent.h"
#include "Engine/CollisionProfile.h"
#include <Runtime\Engine\Classes\Kismet\GameplayStatics.h>
#include <Runtime\Engine\Public\Net\UnrealNetwork.h>
#include <Totem\TotemPlayerController.h>

// Sets default values
AShooterWeapon::AShooterWeapon()
{
	BurstCounter = 0;
	bIsEquipped = false;
	bPendingEquip = false;
	bPendingReload = false;
	bRefiring = false;
	LastFireTime = 0.0f;
	bWantsToFire = false;
}

void AShooterWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AShooterWeapon, BurstCounter, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AShooterWeapon, bPendingReload, COND_SkipOwner);
}

void AShooterWeapon::StartFire()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStartFire();
	}

	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}

void AShooterWeapon::StopFire()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStopFire();
	}

	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}

void AShooterWeapon::StartFireAlt()
{
}

void AShooterWeapon::StopFireAlt()
{
}

void AShooterWeapon::StartReload(bool bFromReplication)
{

}

void AShooterWeapon::StopReload()
{

}

void AShooterWeapon::ReloadWeapon()
{

}

void AShooterWeapon::FireWeapon()
{
}

void AShooterWeapon::OnRep_BurstCounter()
{
	if (BurstCounter > 0)
	{
		SimulateWeaponFire();
	}
	else
	{
		StopSimulatingWeaponFire();
	}
}

void AShooterWeapon::OnRep_Reload()
{
	if (bPendingReload)
	{
		StartReload(true);
	}
	else
	{
		StopReload();
	}
}

bool AShooterWeapon::ServerStartFire_Validate()
{
	return true;
}

void AShooterWeapon::ServerStartFire_Implementation()
{
	StartFire();
}

bool AShooterWeapon::ServerStopFire_Validate()
{
	return true;
}

void AShooterWeapon::ServerStopFire_Implementation()
{
	StopFire();
}

bool AShooterWeapon::ServerStartReload_Validate()
{
	return true;
}

void AShooterWeapon::ServerStartReload_Implementation()
{
	StartReload();
}

bool AShooterWeapon::ServerStopReload_Validate()
{
	return true;
}

void AShooterWeapon::ServerStopReload_Implementation()
{
	StopReload();
}

void AShooterWeapon::ClientStartReload_Implementation()
{
	StartReload();
}

bool AShooterWeapon::CanFire() const
{
	bool bCanFire = MyPawn && MyPawn->IsEnabled;
	bool bStateOKToFire = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return ((bCanFire == true) && (bStateOKToFire == true) && (bPendingReload == false));
}

bool AShooterWeapon::CanReload() const
{
	bool bCanReload = (!MyPawn || /*MyPawn->CanReload()*/ true);
	bool bGotAmmo = CurrentAmmo < MaxAmmo || bInfiniteAmmo;
	bool bStateOKToReload = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return ((bCanReload == true) && (bGotAmmo == true) && (bStateOKToReload == true));
}

void AShooterWeapon::DetermineWeaponState()
{
	EWeaponState NewState = EWeaponState::Idle;

	if (bIsEquipped)
	{
		if (bPendingReload)
		{
			if (CanReload() == false)
			{
				NewState = CurrentState;
			}
			else
			{
				NewState = EWeaponState::Reloading;
			}
		}
		else if ((bPendingReload == false) && (bWantsToFire == true) && (CanFire() == true))
		{
			NewState = EWeaponState::Firing;
		}
	}
	else if (bPendingEquip)
	{
		NewState = EWeaponState::Equipping;
	}

	SetWeaponState(NewState);
}

void AShooterWeapon::SetWeaponState(EWeaponState NewState)
{
	const EWeaponState PrevState = CurrentState;

	if (PrevState == EWeaponState::Firing && NewState != EWeaponState::Firing)
	{
		OnBurstFinished();
	}

	CurrentState = NewState;

	if (PrevState != EWeaponState::Firing && NewState == EWeaponState::Firing)
	{
		OnBurstStarted();
	}
}

void AShooterWeapon::OnBurstStarted()
{
	// Start firing, can be delayed to satisfy TimeBetweenShots
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 && TimeBetweenShots > 0.0f &&
		LastFireTime + TimeBetweenShots > GameTime)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &AShooterWeapon::HandleFiring, LastFireTime + TimeBetweenShots - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void AShooterWeapon::OnBurstFinished()
{
	// Stop firing FX on remote clients
	BurstCounter = 0;

	// Stop firing FX locally, unless it's a dedicated server
	if (GetNetMode() != NM_DedicatedServer)
	{
		StopSimulatingWeaponFire();
	}

	GetWorldTimerManager().ClearTimer(TimerHandle_HandleFiring);
	bRefiring = false;
}

bool AShooterWeapon::ServerHandleFiring_Validate()
{
	return true;
}

void AShooterWeapon::ServerHandleFiring_Implementation()
{
	HandleFiring();

	if ((CurrentAmmo > 0 || bInfiniteAmmo) && CanFire()) {
		UseAmmo();
		BurstCounter++;
	}
}

void AShooterWeapon::ApplyKnockback() {
	if (PawnPrimitiveComponent) {
		FVector Location = FMath::Lerp(PawnPrimitiveComponent->GetComponentLocation(), MuzzleLocation, KnockbackPositionLerp);
		FVector Direction = -PawnPrimitiveComponent->GetForwardVector();
		Direction.Z = 0.f;
		Direction.Normalize();
		PawnPrimitiveComponent->AddImpulseAtLocation(Direction * KnockbackForce, Location);
	}
}

void AShooterWeapon::HandleFiring()
{
	if ((CurrentAmmo > 0 || bInfiniteAmmo) && CanFire())
	{
		if (GetNetMode() != NM_DedicatedServer)
		{
			SimulateWeaponFire();
		}

		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			DetermineMuzzleTransform();

			FireWeapon();

			UseAmmo();

			ApplyKnockback();

			// update firing FX on remote clients if function was called on server
			BurstCounter++;
		}
	}
	else if (CanReload())
	{
		StartReload();
	}
	
	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		if (CurrentAmmo == 0 && !bRefiring)
		{
			// Out of ammo
			OutOfAmmoEffects();

			ATotemPlayerController* PC = Cast<ATotemPlayerController>(MyPawn->Controller);
			if (PC)
			{
				PC->NotifyOutOfAmmo();
			}

			// stop weapon fire FX, but stay in Firing state
			if (BurstCounter > 0)
			{
				OnBurstFinished();
			}
		}
	}

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		// Local client will notify server
		if (GetLocalRole() < ROLE_Authority)
		{
			ServerHandleFiring();
		}

		// Reload after firing last round
		if (CurrentAmmo <= 0 && CanReload())
		{
			StartReload();
		}

		// setup refire timer
		bRefiring = (CurrentState == EWeaponState::Firing && TimeBetweenShots > 0.0f);
		if (bRefiring)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &AShooterWeapon::HandleFiring, TimeBetweenShots, false);
		}
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
}

FVector AShooterWeapon::GetAdjustedAim() const
{
	APlayerController* const PlayerController = GetInstigator() ? Cast<APlayerController>(GetInstigator()->Controller) : NULL;
	FVector FinalAim = FVector::ZeroVector;
	// If we have a player controller use it for the aim
	if (PlayerController)
	{
		//FVector CamLoc;
		//FRotator CamRot;
		//PlayerController->GetPlayerViewPoint(CamLoc, CamRot);
		//FinalAim = CamRot.Vector();
		FinalAim = PlayerController->GetControlRotation().Vector();
	}
	/*else if (GetInstigator())
	{
		// Now see if we have an AI controller - we will want to get the aim from there if we do
		AShooterAIController* AIController = MyPawn ? Cast<AShooterAIController>(MyPawn->Controller) : NULL;
		if (AIController != NULL)
		{
			FinalAim = AIController->GetControlRotation().Vector();
		}
		else
		{
			FinalAim = GetInstigator()->GetBaseAimRotation().Vector();
		}
	}*/

	return FinalAim;
}

FVector AShooterWeapon::GetCameraDamageStartLocation(const FVector& AimDir) const
{
	APlayerController* PC = MyPawn ? Cast<APlayerController>(MyPawn->Controller) : NULL;
	//AShooterAIController* AIPC = MyPawn ? Cast<AShooterAIController>(MyPawn->Controller) : NULL;
	FVector OutStartTrace = FVector::ZeroVector;

	if (PC)
	{
		// use player's camera
		FRotator UnusedRot;
		PC->GetPlayerViewPoint(OutStartTrace, UnusedRot);

		// Adjust trace so there is nothing blocking the ray between the camera and the pawn, and calculate distance from adjusted start
		OutStartTrace = OutStartTrace + AimDir * ((GetInstigator()->GetActorLocation() - OutStartTrace) | AimDir);
	}
	/*else if (AIPC)
	{
		OutStartTrace = GetMuzzleLocation();
	}*/

	return OutStartTrace;
}

FHitResult AShooterWeapon::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const
{
	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());

	FHitResult Hit(ForceInit);
	// TODO: Replace ECC_GameTraceChannel1  with declaration
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECC_GameTraceChannel1, TraceParams);

	return Hit;
}

void AShooterWeapon::SimulateWeaponFire()
{
	if (GetLocalRole() == ROLE_Authority && CurrentState != EWeaponState::Firing)
	{
		return;
	}

	WeaponEffects();
}

void AShooterWeapon::StopSimulatingWeaponFire()
{
	StopWeaponEffects();
}

FVector AShooterWeapon::GetMuzzleLocation()
{
	return MuzzleLocation;
}

void AShooterWeapon::WeaponEffects_Implementation() {

}

void AShooterWeapon::StopWeaponEffects_Implementation() {

}

void AShooterWeapon::DetermineMuzzleTransform_Implementation() {
	MuzzleLocation = GetActorLocation();
}