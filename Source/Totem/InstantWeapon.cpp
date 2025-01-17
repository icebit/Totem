#include "InstantWeapon.h"
#include "Engine/CollisionProfile.h"
#include <Runtime\Engine\Public\Net\UnrealNetwork.h>
#include "TotemPlayerController.h"

AInstantWeapon::AInstantWeapon()
{
	CurrentFiringSpread = 0.0f;
	AllowedViewDotHitDir = 0.8f;
	ClientSideHitLeeway = 200.f;
	WeaponRange = 10000.0f;
	FiringSpreadIncrement = 1.f;
	ImpactLocationNudge = 8.f;
	DamageType = UDamageType::StaticClass();
}

float AInstantWeapon::GetCurrentSpread() const
{
	float FinalSpread = WeaponSpread + CurrentFiringSpread;
	/*if (MyPawn && MyPawn->GetIsAiming())
	{
		FinalSpread *= AimingSpreadMod;
	}*/

	return FinalSpread;
}

float AInstantWeapon::GetCurrentFiringSpreadPercentage() const
{
	float FinalSpread = CurrentFiringSpread;
	/*if (MyPawn && MyPawn->GetIsAiming())
	{
		FinalSpread *= AimingSpreadMod;
	}*/

	return FinalSpread / FiringSpreadMax;
 
}

bool AInstantWeapon::ServerNotifyHit_Validate(FVector Origin, const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread)
{
	return true;
}

void AInstantWeapon::ServerNotifyHit_Implementation(FVector Origin, const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread)
{
	// TODO: Fix this code by fixing get muzzle location function

	/*const float WeaponAngleDot = FMath::Abs(FMath::Sin(ReticleSpread * PI / 180.f));

	// if we have an instigator, calculate dot between the view and the shot
	if (GetInstigator() && (Impact.GetActor() || Impact.bBlockingHit))
	{
		const FVector Origin = GetMuzzleLocation();
		const FVector ViewDir = (Impact.Location - Origin).GetSafeNormal();

		// is the angle between the hit and the view within allowed limits (limit + weapon max angle)
		const float ViewDotHitDir = FVector::DotProduct(GetInstigator()->GetViewRotation().Vector(), ViewDir);
		if (ViewDotHitDir > AllowedViewDotHitDir - WeaponAngleDot)
		{
			if (CurrentState != EWeaponState::Idle)
			{
				if (Impact.GetActor() == NULL)
				{
					if (Impact.bBlockingHit)
					{
						ProcessInstantHit_Confirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
					}
				}
				// assume it told the truth about static things because the don't move and the hit 
				// usually doesn't have significant gameplay implications
				else if (Impact.GetActor()->IsRootComponentStatic() || Impact.GetActor()->IsRootComponentStationary())
				{
					ProcessInstantHit_Confirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
				}
				else
				{
					// Get the component bounding box
					const FBox HitBox = Impact.GetActor()->GetComponentsBoundingBox();

					// calculate the box extent, and increase by a leeway
					FVector BoxExtent = 0.5 * (HitBox.Max - HitBox.Min);
					BoxExtent *= ClientSideHitLeeway;

					// avoid precision errors with really thin objects
					BoxExtent.X = FMath::Max(20.0f, BoxExtent.X);
					BoxExtent.Y = FMath::Max(20.0f, BoxExtent.Y);
					BoxExtent.Z = FMath::Max(20.0f, BoxExtent.Z);

					// Get the box center
					const FVector BoxCenter = (HitBox.Min + HitBox.Max) * 0.5;

					// if we are within client tolerance
					if (FMath::Abs(Impact.Location.Z - BoxCenter.Z) < BoxExtent.Z &&
						FMath::Abs(Impact.Location.X - BoxCenter.X) < BoxExtent.X &&
						FMath::Abs(Impact.Location.Y - BoxCenter.Y) < BoxExtent.Y)
					{
						ProcessInstantHit_Confirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
					}
					else
					{
						UE_LOG(LogTemp, Log, TEXT("%s Rejected client side hit of %s (outside bounding box tolerance)"), *GetNameSafe(this), *GetNameSafe(Impact.GetActor()));
					}
				}
			}
		}
		else if (ViewDotHitDir <= AllowedViewDotHitDir)
		{
			UE_LOG(LogTemp, Log, TEXT("%s Rejected client side hit of %s (facing too far from the hit direction)"), *GetNameSafe(this), *GetNameSafe(Impact.GetActor()));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("%s Rejected client side hit of %s"), *GetNameSafe(this), *GetNameSafe(Impact.GetActor()));
		}
	}*/

	ProcessInstantHit_Confirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
}

bool AInstantWeapon::ServerNotifyMiss_Validate(FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread)
{
	return true;
}

void AInstantWeapon::ServerNotifyMiss_Implementation(FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread)
{
}

void AInstantWeapon::ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread)
{
	if (MyPawn && MyPawn->IsLocallyControlled() && GetNetMode() == NM_Client)
	{
		// if we're a client and we've hit something that is being controlled by the server
		if (Impact.GetActor() && Impact.GetActor()->GetRemoteRole() == ROLE_Authority)
		{
			// notify the server of the hit
			ServerNotifyHit(Origin, Impact, ShootDir, RandomSeed, ReticleSpread);
		}
		else if (Impact.GetActor() == NULL)
		{
			if (Impact.bBlockingHit)
			{
				// notify the server of the hit
				ServerNotifyHit(Origin, Impact, ShootDir, RandomSeed, ReticleSpread);
			}
			else
			{
				// notify server of the miss
				ServerNotifyMiss(ShootDir, RandomSeed, ReticleSpread);
			}
		}
	}

	// Notify hit if not dedicated server
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (Impact.GetActor() != NULL) {
			if (Impact.GetActor()->CanBeDamaged()) {
				ATotemPlayerController* PC = Cast<ATotemPlayerController>(MyPawn->Controller);
				if (PC)
				{
					PC->NotifyHitEnemy();
				}
			}
		}
	}

	// process a confirmed hit
	ProcessInstantHit_Confirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
}

void AInstantWeapon::ProcessInstantHit_Confirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread)
{
	// handle damage
	if (ShouldDealDamage(Impact.GetActor()))
	{
		DealDamage(Impact, ShootDir);
	}

	// play FX on remote clients
	if (GetLocalRole() == ROLE_Authority)
	{
		HitNotify.Origin = Origin;
		HitNotify.RandomSeed = RandomSeed;
		HitNotify.ReticleSpread = ReticleSpread;
	}

	// play FX locally
	if (GetNetMode() != NM_DedicatedServer)
	{
		const FVector EndTrace = Origin + ShootDir * WeaponRange;
		const FVector EndPoint = Impact.GetActor() ? Impact.ImpactPoint : EndTrace;

		const FVector NudgedImpactLocation = Impact.ImpactPoint + Impact.ImpactNormal * ImpactLocationNudge;

		if(Impact.bBlockingHit)
		{
			SpawnImpactEffects(NudgedImpactLocation);
		}
		SpawnTrailEffects(Origin, EndPoint);
	}
}

bool AInstantWeapon::ShouldDealDamage(AActor* TestActor) const
{	
	// If we're an actor on the server, or the actor's role is authoritative, we should register damage
	if (TestActor)
	{
		if (GetNetMode() != NM_Client ||
			TestActor->GetLocalRole() == ROLE_Authority ||
			TestActor->GetTearOff())
		{
			return true;
		}
	}

	return false;
}

float AInstantWeapon::DealDamage(const FHitResult& Impact, const FVector& ShootDir)
{
	FPointDamageEvent PointDmg;
	PointDmg.DamageTypeClass = DamageType;
	PointDmg.HitInfo = Impact;
	PointDmg.ShotDirection = ShootDir;
	PointDmg.Damage = Damage;

	return Impact.GetActor()->TakeDamage(PointDmg.Damage, PointDmg, MyPawn->Controller, this);
}

void AInstantWeapon::FireWeapon()
{
	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	const float CurrentSpread = GetCurrentSpread();
	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);

	const FVector AimDir = GetAdjustedAim();
	const FVector StartTrace = GetCameraDamageStartLocation(AimDir);
	const FVector ShootDir = WeaponRandomStream.VRandCone(AimDir, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDir * WeaponRange;

	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());
	FHitResult Hit(ForceInit);
	GetWorld()->SweepSingleByChannel(Hit, StartTrace, EndTrace, FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(BulletRadius), TraceParams);

	ProcessInstantHit(Hit, StartTrace, ShootDir, RandomSeed, CurrentSpread);

	CurrentFiringSpread = FMath::Min(FiringSpreadMax, CurrentFiringSpread + FiringSpreadIncrement);
}

void AInstantWeapon::OnBurstFinished()
{
	Super::OnBurstFinished();

	CurrentFiringSpread = 0.0f;
}

void AInstantWeapon::OnRep_HitNotify()
{
	SimulateInstantHit(HitNotify.Origin, HitNotify.RandomSeed, HitNotify.ReticleSpread);
}

void AInstantWeapon::SimulateInstantHit(const FVector& Origin, int32 RandomSeed, float ReticleSpread)
{
	FRandomStream WeaponRandomStream(RandomSeed);
	const float ConeHalfAngle = FMath::DegreesToRadians(ReticleSpread * 0.5f);

	const FVector StartTrace = Origin;
	const FVector AimDir = GetAdjustedAim();
	const FVector ShootDir = WeaponRandomStream.VRandCone(AimDir, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDir * WeaponRange;

	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());

	FHitResult Impact(ForceInit);
	// TODO: Replace ECC_GameTraceChannel1  with declaration
	//FHitResult Impact = WeaponTrace(StartTrace, EndTrace);

	//GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECC_GameTraceChannel1, TraceParams);
	GetWorld()->SweepSingleByChannel(Impact, StartTrace, EndTrace, FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(BulletRadius), TraceParams);

	const FVector EndPoint = Impact.GetActor() ? Impact.ImpactPoint : EndTrace;
	const FVector NudgedImpactLocation = Impact.ImpactPoint + Impact.ImpactNormal * ImpactLocationNudge;

	if(Impact.bBlockingHit)
	{
		SpawnImpactEffects(NudgedImpactLocation);
	}
	SpawnTrailEffects(Origin, EndPoint);
}

void AInstantWeapon::SpawnImpactEffects_Implementation(FVector NudgedImpactLocation)
{
}

void AInstantWeapon::SpawnTrailEffects_Implementation(FVector StartPoint, FVector EndPoint)
{
}

void AInstantWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AInstantWeapon, HitNotify, COND_SkipOwner);
}
