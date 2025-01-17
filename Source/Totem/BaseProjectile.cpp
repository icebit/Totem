#include "BaseProjectile.h"
#include "Totem.h"
#include "ProjectileWeapon.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include <Runtime\Engine\Public\Net\UnrealNetwork.h>

#include "TotemPlayerController.h"

ABaseProjectile::ABaseProjectile()
{
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->AlwaysLoadOnClient = true;
	CollisionComp->AlwaysLoadOnServer = true;
	CollisionComp->bTraceComplexOnMove = true;
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(COLLISION_PROJECTILE);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	RootComponent = CollisionComp;

	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	MovementComp->UpdatedComponent = CollisionComp;
	MovementComp->InitialSpeed = 2000.0f;
	MovementComp->MaxSpeed = 2000.0f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->ProjectileGravityScale = 0.f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bExploded = false;
	SetReplicatingMovement(true);
}

void ABaseProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (GetLocalRole() == ROLE_Authority) {
		CollisionComp->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnImpact);
	}
	CollisionComp->MoveIgnoreActors.Add(GetInstigator());

	OwnerWeapon = Cast<AProjectileWeapon>(GetOwner());
	SetLifeSpan(ProjectileLifeSpan);
	MyController = GetInstigatorController();
}

// To be deleted
void ABaseProjectile::InitVelocity(FVector& ShootDirection)
{
}

void ABaseProjectile::OnImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (GetLocalRole() == ROLE_Authority && !bExploded)
	{
		if (OtherActor)
		{
			UGameplayStatics::ApplyPointDamage(OtherActor, Damage, NormalImpulse, Hit, GetInstigator()->Controller, this, DamageType);

			// Notify hit if not dedicated server
			if (GetNetMode() != NM_DedicatedServer)
			{
				if (OtherActor->CanBeDamaged()) {
					ATotemPlayerController* PC = Cast<ATotemPlayerController>(OwnerWeapon->MyPawn->Controller);
					if (PC)
					{
						PC->NotifyHitEnemy();
					}
				}
			}
		}
 
		// Hit effects
		bExploded = true;
		HitEffects(Hit);
		
		DisableAndDestroy();
	}
}

void ABaseProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseProjectile, bExploded);
}

void ABaseProjectile::HitEffects_Implementation(const FHitResult& Hit)
{
}

void ABaseProjectile::OnRep_Exploded()
{
	FVector ProjDirection = GetActorForwardVector();

	const FVector StartTrace = GetActorLocation() - ProjDirection * 200;
	const FVector EndTrace = GetActorLocation() + ProjDirection * 150;
	FHitResult Impact;

	if (!GetWorld()->LineTraceSingleByChannel(Impact, StartTrace, EndTrace, COLLISION_PROJECTILE, FCollisionQueryParams(SCENE_QUERY_STAT(ProjClient), true, GetInstigator())))
	{
		// failsafe
		Impact.ImpactPoint = GetActorLocation();
		Impact.ImpactNormal = -ProjDirection;
	}

	bExploded = true;
	HitEffects(Impact);
}

void ABaseProjectile::DisableAndDestroy()
{
	UAudioComponent* ProjAudioComp = FindComponentByClass<UAudioComponent>();
	if (ProjAudioComp && ProjAudioComp->IsPlaying())
	{
		ProjAudioComp->FadeOut(0.1f, 0.f);
	}

	MovementComp->StopMovementImmediately();
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetLifeSpan(PostHitLifeSpan);
}

/*void ABaseProjectile::OnRep_Exploded()
{
	FVector ProjDirection = GetActorForwardVector();

	const FVector StartTrace = GetActorLocation() - ProjDirection * 200;
	const FVector EndTrace = GetActorLocation() + ProjDirection * 150;
	FHitResult Impact;

	if (!GetWorld()->LineTraceSingleByChannel(Impact, StartTrace, EndTrace, COLLISION_PROJECTILE, FCollisionQueryParams(SCENE_QUERY_STAT(ProjClient), true, GetInstigator())))
	{
		// failsafe
		Impact.ImpactPoint = GetActorLocation();
		Impact.ImpactNormal = -ProjDirection;
	}

	Explode(Impact);
}*/

/*void ABaseProjectile::PostNetReceiveVelocity(const FVector& NewVelocity)
{
	if (MovementComp)
	{
		MovementComp->Velocity = NewVelocity;
	}
}*/