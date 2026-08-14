// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileWeapon.h"
#include "BaseProjectile.h"
#include "Camera/CameraComponent.h"
#include "TotemPawn.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AProjectileWeapon::AProjectileWeapon()
{
	bPredictLocally = true;
	MaxMuzzleDeviation = 300.0f;
	MaxPredictionTime = 0.2f;
	AbsurdMuzzleDeviation = 5000.0f;
}

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

	if (bPredictLocally && GetLocalRole() < ROLE_Authority)
	{
		SpawnProjectile(Origin, ShootDir, true);
	}

	ServerFireProjectile(Origin, ShootDir);
}

bool AProjectileWeapon::ServerFireProjectile_Validate(FVector Origin, FVector_NetQuantizeNormal ShootDir)
{
	if (Origin.ContainsNaN() || ShootDir.ContainsNaN())
	{
		return false;
	}

	// Quantization means this will not be exactly unit length
	if (FMath::Abs(ShootDir.SizeSquared() - 1.0f) > 0.1f)
	{
		return false;
	}

	if (MyPawn && FVector::Dist(Origin, MyPawn->GetActorLocation()) > AbsurdMuzzleDeviation)
	{
		return false;
	}

	return true;
}

void AProjectileWeapon::ServerFireProjectile_Implementation(FVector Origin, FVector_NetQuantizeNormal ShootDir)
{
	if (!MyPawn || !ProjectileClass)
	{
		return;
	}

	if (!AcceptServerFire())
	{
		return;
	}

	// The muzzle transform is only computed for locally controlled pawns, so the server has no
	// trustworthy muzzle of its own. Anchor plausibility on the pawn and fall back to it.
	const FVector PawnLocation = MyPawn->GetActorLocation();
	FVector SpawnOrigin = Origin;
	if (FVector::Dist(SpawnOrigin, PawnLocation) > MaxMuzzleDeviation)
	{
		SpawnOrigin = PawnLocation;
	}

	const float PredictionTime = GetShooterPredictionTime();
	if (PredictionTime > 0.0f)
	{
		const ABaseProjectile* Defaults = ProjectileClass->GetDefaultObject<ABaseProjectile>();
		const float LaunchSpeed = Defaults ? Defaults->GetLaunchSpeed() : 0.0f;
		const float AdvanceDistance = LaunchSpeed * PredictionTime;

		if (AdvanceDistance > 0.0f)
		{
			const FVector Target = SpawnOrigin + ShootDir * AdvanceDistance;
			FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(ProjectileCatchUp), true, GetInstigator());

			FHitResult Blocked(ForceInit);
			if (GetWorld()->LineTraceSingleByChannel(Blocked, SpawnOrigin, Target, ECC_GameTraceChannel1, TraceParams))
			{
				// Stop short of geometry rather than advancing through a wall
				SpawnOrigin = Blocked.ImpactPoint - ShootDir * 10.0f;
			}
			else
			{
				SpawnOrigin = Target;
			}
		}
	}

	SpawnProjectile(SpawnOrigin, ShootDir, false);
}

ABaseProjectile* AProjectileWeapon::SpawnProjectile(const FVector& Origin, const FVector& ShootDir, bool bCosmetic)
{
	if (!ProjectileClass)
	{
		return nullptr;
	}

	const FTransform SpawnTM(ShootDir.Rotation(), Origin);
	ABaseProjectile* Projectile = Cast<ABaseProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ProjectileClass, SpawnTM));
	if (!Projectile)
	{
		return nullptr;
	}

	Projectile->SetInstigator(GetInstigator());
	Projectile->SetOwner(this);

	if (bCosmetic)
	{
		Projectile->MakeCosmetic();
	}

	UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
	return Projectile;
}

float AProjectileWeapon::GetShooterPredictionTime() const
{
	// APlayerController::GetPredictionTime returns 0 on a dedicated server, so derive it from
	// the shooter's ping directly. ExactPing is round trip in milliseconds and server side only.
	const APlayerState* ShooterState = MyPawn ? MyPawn->GetPlayerState() : nullptr;
	if (!ShooterState)
	{
		return 0.0f;
	}

	const float OneWaySeconds = ShooterState->ExactPing * 0.0005f;
	return FMath::Clamp(OneWaySeconds, 0.0f, MaxPredictionTime);
}
