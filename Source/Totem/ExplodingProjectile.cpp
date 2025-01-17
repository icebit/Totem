// Fill out your copyright notice in the Description page of Project Settings.


#include "ExplodingProjectile.h"
#include <Runtime\Engine\Classes\Kismet\GameplayStatics.h>

void AExplodingProjectile::OnImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (GetLocalRole() == ROLE_Authority && !bExploded)
	{
		// effects and damage origin shouldn't be placed inside mesh at impact point
		const FVector NudgedImpactLocation = Hit.ImpactPoint + Hit.ImpactNormal * 10.0f;

		if (Damage > 0 && ExplosionRadius > 0 && DamageType)
		{
			UGameplayStatics::ApplyRadialDamage(this, Damage, NudgedImpactLocation, ExplosionRadius, DamageType, TArray<AActor*>(), this, MyController.Get());
		}
		// Hit effects
		bExploded = true;
		HitEffects(Hit);

		DisableAndDestroy();
	}
}
