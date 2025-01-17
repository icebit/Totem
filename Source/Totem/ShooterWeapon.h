// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseWeapon.h"
#include "CoreMinimal.h"
#include "TotemPawn.h"
#include "BaseProjectile.h"
#include "GameFramework/Actor.h"
#include "ShooterWeapon.generated.h"

UENUM()
enum EWeaponState {
	Idle,
	Firing,
	Reloading,
	Equipping,
};

UCLASS(Abstract, Blueprintable)
class TOTEM_API AShooterWeapon : public ABaseWeapon
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AShooterWeapon();

	// [local + server] start weapon fire
	void StartFire() override;

	// [local + server] stop weapon fire
	void StopFire() override;

	// [local + server] start weapon fire alt
	void StartFireAlt() override;

	// [local + server] stop weapon fire alt
	void StopFireAlt() override;

	// Knockback force
	UPROPERTY(EditDefaultsOnly, Category = Fire)
	float KnockbackForce;

	// Interpolation between center of totem and applied knockback position
	UPROPERTY(EditDefaultsOnly, Category = Fire)
	float KnockbackPositionLerp;

	// Set to zero if the weapon is not automatic
	UPROPERTY(Category = Charge, EditAnywhere)
	float TimeBetweenShots;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

protected:	
	// Burst counter, used for replicating fire events to remote clients
	UPROPERTY(Transient, ReplicatedUsing = OnRep_BurstCounter, BlueprintReadWrite)
	int32 BurstCounter;

	UFUNCTION()
	void OnRep_BurstCounter();

	UFUNCTION()
	void ApplyKnockback();

	// [local + server] firing started
	virtual void OnBurstStarted();

	// [local + server] firing finished
	virtual void OnBurstFinished();

	// [server] fire & update ammo
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerHandleFiring();

	// [local + server] handle weapon fire
	void HandleFiring();

	FVector GetAdjustedAim() const;

	FVector GetCameraDamageStartLocation(const FVector& AimDir) const;

	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const;

	UPROPERTY(BlueprintReadWrite, Category = Fire)
	FVector MuzzleLocation;

	// Get location of weapon's muzzle
	UFUNCTION(Category = Fire)
	FVector GetMuzzleLocation();

	UPROPERTY(Transient, ReplicatedUsing = OnRep_Reload)
	bool bPendingReload;

	UFUNCTION()
	void OnRep_Reload();

	bool bRefiring;

	bool bWantsToFire;

	// Time of last successful weapon fire
	float LastFireTime;

	EWeaponState CurrentState;

	void DetermineWeaponState() override;

	void SetWeaponState(EWeaponState NewState);

	// [all] start weapon reload
	virtual void StartReload(bool bFromReplication = false);

	// [local + server] interrupt weapon reload
	virtual void StopReload();

	// [server] performs actual reload
	virtual void ReloadWeapon();

	virtual void FireWeapon();
	
	// Triggered reload from server
	UFUNCTION(Reliable, Client)
	void ClientStartReload();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartFire();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopFire();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartReload();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopReload();

	// Check if weapon can fire
	bool CanFire() const;

	// Check if weapon can be reloaded
	bool CanReload() const;

	// Called in network play to do the cosmetic fx for firing
	virtual void SimulateWeaponFire();

	// Called in network play to stop cosmetic fx (e.g. for a looping shot).
	virtual void StopSimulatingWeaponFire();

	// Blueprint implementable function for fx
	UFUNCTION(Category = Fire, BlueprintNativeEvent)
	void WeaponEffects();

	// Blueprint implementable function to stop fx
	UFUNCTION(Category = Fire, BlueprintNativeEvent)
	void StopWeaponEffects();

	// Blueprint implementable function to stop fx
	UFUNCTION(Category = Fire, BlueprintNativeEvent)
	void DetermineMuzzleTransform();

	FTimerHandle TimerHandle_HandleFiring;
};
