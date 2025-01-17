// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterWeapon.h"
#include "InstantWeapon.generated.h"

USTRUCT()
struct FInstantHitInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector Origin;

	UPROPERTY()
	float ReticleSpread;

	UPROPERTY()
		int32 RandomSeed;
};

UCLASS()
class TOTEM_API AInstantWeapon : public AShooterWeapon
{
	GENERATED_BODY()

	AInstantWeapon();

	UPROPERTY(EditAnywhere, Category = Effects)
	float ImpactLocationNudge;

	UPROPERTY(EditAnywhere, Category = Spread)
	float WeaponSpread;

	UPROPERTY(EditAnywhere, Category = Spread)
	float AimingSpreadMod;

	UPROPERTY(EditAnywhere, Category = Spread)
	float FiringSpreadMax;

	UPROPERTY(EditAnywhere, Category = Spread)
	float FiringSpreadIncrement;

	UPROPERTY(EditAnywhere, Category = HitVerification)
	float AllowedViewDotHitDir;

	UPROPERTY(EditAnywhere, Category = HitVerification)
	float ClientSideHitLeeway;

	UPROPERTY(EditAnywhere, Category = Fire)
	float WeaponRange;

	UPROPERTY(EditAnywhere, Category = Fire)
	float BulletRadius;

	UPROPERTY(EditAnywhere, Category = Damage)
	TSubclassOf<UDamageType> DamageType;
	
	UPROPERTY(EditAnywhere, Category = Damage)
	float Damage;

	// Get current spread
	UFUNCTION(BlueprintCallable)
	float GetCurrentSpread() const;

	UFUNCTION(BlueprintCallable)
	float GetCurrentFiringSpreadPercentage() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

protected:
	// Instant hit notify for replication
	UPROPERTY(Transient, ReplicatedUsing = OnRep_HitNotify)
	FInstantHitInfo HitNotify;

	// Current spread from continuous firing
	UPROPERTY(BlueprintReadOnly)
	float CurrentFiringSpread;

	// Weapon usage

	// Server notified of hit from client to verify
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerNotifyHit(FVector Origin, const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);
	bool ServerNotifyHit_Validate(FVector Origin, const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);
	void ServerNotifyHit_Implementation(FVector Origin, const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);

	// Server notified of miss to show FX
	UFUNCTION(Unreliable, Server, WithValidation)
	void ServerNotifyMiss(FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);
	bool ServerNotifyMiss_Validate(FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);
	void ServerNotifyMiss_Implementation(FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);

	// Process the instant hit and notify the server if necessary
	void ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);

	// Continue processing the instant hit, as if it has been confirmed by the server
	void ProcessInstantHit_Confirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);

	// Check if weapon should deal damage to actor
	bool ShouldDealDamage(AActor* TestActor) const;

	// Handle damage
	float DealDamage(const FHitResult& Impact, const FVector& ShootDir);

	// [local] weapon specific fire implementation
	virtual void FireWeapon() override;

	// [local + server] update spread on firing
	virtual void OnBurstFinished() override;

	// Effects replication
	UFUNCTION()
	void OnRep_HitNotify();

	// Called in network play to do the cosmetic fx
	void SimulateInstantHit(const FVector& Origin, int32 RandomSeed, float ReticleSpread);

	// Spawn effects for impact
	UFUNCTION(Category = Fire, BlueprintNativeEvent)
	void SpawnImpactEffects(FVector NudgedImpactLocation);
	
	UFUNCTION(Category = Fire, BlueprintNativeEvent)
	void SpawnTrailEffects(FVector StartPoint, FVector EndPoint);
};
