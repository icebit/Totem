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

protected:

	// [local] weapon specific fire implementation
	virtual void FireWeapon() override;

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerFireProjectile(FVector Origin, FVector_NetQuantizeNormal ShootDir);
};
