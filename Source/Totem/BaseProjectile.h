#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

UCLASS()
class TOTEM_API ABaseProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABaseProjectile();

	// Initial setup
	virtual void PostInitializeComponents() override;

	// Setup velocity
	void InitVelocity(FVector& ShootDirection);

	// Handle hit
	UFUNCTION()
	virtual void OnImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// Life time
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	float ProjectileLifeSpan;

	// Life time
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	float PostHitLifeSpan;

	// Type of damage
	UPROPERTY(EditDefaultsOnly, Category = Damage)
	TSubclassOf<UDamageType> DamageType;

	// Amount of damage
	UPROPERTY(EditDefaultsOnly, Category = Damage)
	float Damage;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
protected:
	// Movement component
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	class UProjectileMovementComponent* MovementComp;

	// Collisions
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile)
	class USphereComponent* CollisionComp;

	class AProjectileWeapon* OwnerWeapon;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_Exploded)
	bool bExploded;

	UFUNCTION()
	void OnRep_Exploded();

	UFUNCTION(BlueprintNativeEvent, Category = Projectile)
	void HitEffects(const FHitResult& Hit);

	// Effects for explosion
	//UPROPERTY(EditDefaultsOnly, Category = Effects)
	//TSubclassOf<class AShooterExplosionEffect> ExplosionTemplate;

	// Controller that fired me (cache for damage calculations) */
	TWeakObjectPtr<AController> MyController;

	// Shutdown projectile and prepare for destruction
	void DisableAndDestroy();
};
