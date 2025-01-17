// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileWeapon.h"
#include "Camera/CameraComponent.h"
#include "TotemPawn.h"
#include <Runtime\Engine\Classes\Kismet\GameplayStatics.h>
#include <Runtime\Engine\Classes\Kismet\KismetMathLibrary.h>


void AProjectileWeapon::FireWeapon()
{
	FVector ShootDir = MyPawn->GetControlRotation().Vector();
	FVector Origin = GetMuzzleLocation();

	const float ProjectileAdjustRange = 10000.0f;
	const FVector StartTrace = Origin;
	const FVector EndTrace = StartTrace + ShootDir * ProjectileAdjustRange;

	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());

	FHitResult Hit(ForceInit);
	// TODO: Replace ECC_GameTraceChannel1  with declaration
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECC_GameTraceChannel1, TraceParams);
	
	if (Hit.bBlockingHit) {
		FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(Origin, Hit.ImpactPoint);
		ShootDir = Rotation.Vector();
	}

	ServerFireProjectile(Origin, ShootDir);
}

bool AProjectileWeapon::ServerFireProjectile_Validate(FVector Origin, FVector_NetQuantizeNormal ShootDir)
{
	return true;
}

void AProjectileWeapon::ServerFireProjectile_Implementation(FVector Origin, FVector_NetQuantizeNormal ShootDir)
{
	FTransform SpawnTM(ShootDir.Rotation(), Origin);
	ABaseProjectile* Projectile = Cast<ABaseProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ProjectileClass, SpawnTM));
	if (Projectile)
	{
		Projectile->SetInstigator(GetInstigator());
		Projectile->SetOwner(this);

		UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
	}
}
