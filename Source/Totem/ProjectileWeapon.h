// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterWeapon.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class TOTEM_API AProjectileWeapon : public AShooterWeapon
{
	GENERATED_BODY()
	
public:
	AProjectileWeapon();

	// Projectile class
	UPROPERTY(EditAnywhere, Category = Projectile)
	TSubclassOf<class ABaseProjectile> ProjectileClass;

	/*
	// Damage at impact point
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	int32 ExplosionDamage;

	// Radius of damage
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float ExplosionRadius;
	*/

	// Spawn a non-replicated, non-damaging projectile locally so the shooter sees it immediately
	UPROPERTY(EditDefaultsOnly, Category = Netcode)
	bool bPredictLocally;

	// How far the client-reported muzzle may sit from the pawn before the server overrides it
	UPROPERTY(EditDefaultsOnly, Category = Netcode)
	float MaxMuzzleDeviation;

	// Upper bound on how far the server advances a projectile to compensate for the shooter's latency
	UPROPERTY(EditDefaultsOnly, Category = Netcode)
	float MaxPredictionTime;

	// Distance beyond which a reported muzzle is treated as malicious rather than desynced
	UPROPERTY(EditDefaultsOnly, Category = Netcode)
	float AbsurdMuzzleDeviation;

protected:

	// [local] weapon specific fire implementation
	virtual void FireWeapon() override;

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerFireProjectile(FVector Origin, FVector_NetQuantizeNormal ShootDir);

	class ABaseProjectile* SpawnProjectile(const FVector& Origin, const FVector& ShootDir, bool bCosmetic);

	// [server] One-way latency of the shooter, clamped to MaxPredictionTime
	float GetShooterPredictionTime() const;
};
