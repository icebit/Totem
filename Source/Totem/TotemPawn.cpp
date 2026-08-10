// Copyright Epic Games, Inc. All Rights Reserved.

#include "TotemPawn.h"
#include "TotemGameMode.h"
#include "TakeHitInfo.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "Components/AudioComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "BaseWeapon.h"
#include "ShooterWeapon.h"
#include "Engine/StaticMesh.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PIDComponent.h"
#include "TotemPlayerController.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundCue.h"
#include "Materials/MaterialInstanceDynamic.h"

ATotemPawn::ATotemPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	TargetCameraDistance = 84.0f;
	CameraHeight = 450.f;
	LookSpeed = 20.f;

	IsEnabled = true;

	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;

	// Camera defaults
	NormalCameraDistance = 84.0f;
	AimingCameraDistance = 50.0f;
	NormalCameraFOV = 90.0f;
	AimingCameraFOV = 60.0f;
	CameraDistanceLerp = 6.0f;
	CameraFOVLerp = 6.0f;
	MinCameraDistance = 30.0f;
	MaxCameraDistance = 200.0f;
	InFieldOfView = 90.0f;
	NormalSpeed = 1.0f;
	AimingSpeed = 0.5f;
	NormalHorizontalDamping = 0.0f;
	AimingHorizontalDamping = 0.0f;
	NormalVerticalDamping = 0.0f;
	AimingVerticalDamping = 0.0f;

	// Hover defaults
	DesiredHoverHeight = 120.0f;
	MaxHoverHeight = 500.0f;
	HoverForce = 1000000.0f;
	ProportionalCoefficient = 3.5f;
	DerivativeCoefficient = 0.5f;
	PitchStabilization = 5000.0f;
	RollStabilization = 5000.0f;
	YawStabilization = 1000.0f;
	DesiredRollAngle = 0.0f;
	AlignSpeed = 5.0f;
	IsGrounded = false;
	HoverTraceChannel = ECC_GameTraceChannel5;

	// Movement defaults
	MovementForce = 500000.0f;
	AngularDamping = 3.0f;
	SpeedSquaredAtMaxWindVolume = 250000.0f;

	// Dash defaults
	DashForce = 800000.0f;
	DashDuration = 0.3f;
	DashForwardAxisMultiplier = 1.0f;
	DashRightAxisMultiplier = 1.0f;
	DashUpAxisMultiplier = 0.5f;
	DashRotationAxis = EDashRotationAxis::Both;
	IsDashing = false;
	WantsToDash = false;

	// Interact
	MaxInteractDistance = 300.0f;
	IsInteracting = false;

	// Nametag
	NametagHeight = 150.0f;
	bNametagVisible = true;

	// Death
	bFrozen = false;
	RegenerationDelay = 5.0f;
	RegenerationRate = 10.0f;

	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> TotemMesh;
		FConstructorStatics()
			: TotemMesh(TEXT("/Game/Meshes/Totem"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	TotemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TotemMesh0"));
	TotemMesh->SetStaticMesh(ConstructorStatics.TotemMesh.Get());
	TotemMesh->BodyInstance.MassScale = 1.f;
	TotemMesh->OnComponentBeginOverlap.AddDynamic(this, &ATotemPawn::BeginOverlap);
	RootComponent = TotemMesh;

	// Outline mesh (second mesh for team outline)
	TotemOutline = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TotemOutline"));
	TotemOutline->SetupAttachment(TotemMesh);
	TotemOutline->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TotemOutline->SetVisibility(false);

	WeaponMountPoint = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponMountPoint0"));
	WeaponMountPoint->SetupAttachment(TotemMesh);
	WeaponMountPoint->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	// Force application point (center of mass, prevents spin from forces)
	ForceApplication = CreateDefaultSubobject<USceneComponent>(TEXT("ForceApplication"));
	ForceApplication->SetupAttachment(TotemMesh);
	ForceApplication->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	// Dash force point
	DashForcePoint = CreateDefaultSubobject<USceneComponent>(TEXT("DashForcePoint"));
	DashForcePoint->SetupAttachment(TotemMesh);
	DashForcePoint->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	// Hover PID
	HoverPID = CreateDefaultSubobject<UPIDComponent>(TEXT("HoverPID"));
	HoverPID->ProportionalCoefficient = ProportionalCoefficient;
	HoverPID->DerivativeCoefficient = DerivativeCoefficient;

	// Nametag
	Nametag = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Nametag"));
	Nametag->SetupAttachment(TotemMesh);
	Nametag->SetRelativeLocation(FVector(0.f, 0.f, NametagHeight));
	Nametag->SetText(FText::FromString(TEXT("Player")));
	Nametag->SetTextRenderColor(FColor::White);
	Nametag->SetVisibility(true);

	// Ambient loop audio
	AmbientLoop = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientLoop"));
	AmbientLoop->SetupAttachment(TotemMesh);
	AmbientLoop->bAutoActivate = true;
	AmbientLoop->VolumeMultiplier = 0.0f;

	// Spring arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(TotemMesh);
	SpringArm->TargetArmLength = TargetCameraDistance;
	SpringArm->bEnableCameraLag = false;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;

	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->SetFieldOfView(NormalCameraFOV);

	NeutralHoverPhase = FMath::RandRange(0.f, 6.28f);

	Team = -1;
}

void ATotemPawn::SetTargetCameraDistance(float Distance)
{
	TargetCameraDistance = FMath::Clamp(Distance, MinCameraDistance, MaxCameraDistance);
}

void ATotemPawn::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

TArray<class ABaseWeapon*> ATotemPawn::GetInventory()
{
	return Inventory;
}

ABaseWeapon* ATotemPawn::GetWeapon() const
{
	return CurrentWeapon;
}

void ATotemPawn::AddWeapon(ABaseWeapon* Weapon)
{
	if (Weapon && GetLocalRole() == ROLE_Authority)
	{
		Weapon->OnEnterInventory(this);
		Inventory.AddUnique(Weapon);
		OnUpdateInventory();
	}
}

void ATotemPawn::RemoveWeapon(ABaseWeapon* Weapon)
{
	if (Weapon && GetLocalRole() == ROLE_Authority)
	{
		Weapon->OnLeaveInventory();
		Inventory.RemoveSingle(Weapon);
		OnUpdateInventory();
	}
}

ABaseWeapon* ATotemPawn::FindWeapon(TSubclassOf<class ABaseWeapon> WeaponClass)
{
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i] && Inventory[i]->IsA(WeaponClass))
		{
			return Inventory[i];
		}
	}
	return NULL;
}

void ATotemPawn::EquipWeapon(ABaseWeapon* Weapon)
{
	if (Weapon && !IsDead)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			SetCurrentWeapon(Weapon, CurrentWeapon);
		}
		else
		{
			SetCurrentWeapon(Weapon, CurrentWeapon);
			ServerEquipWeapon(Weapon);
		}
	}
}

void ATotemPawn::SetCurrentWeapon(ABaseWeapon* NewWeapon, ABaseWeapon* LastWeapon)
{
	ABaseWeapon* LocalLastWeapon = NULL;

	if (LastWeapon != NULL)
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != CurrentWeapon)
	{
		LocalLastWeapon = CurrentWeapon;
	}

	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnequip();
		bIsFiring = false;
		bIsFiringAlt = false;
	}

	CurrentWeapon = NewWeapon;

	if (NewWeapon)
	{
		NewWeapon->SetMyPawn(this);
		NewWeapon->OnEquip(LastWeapon);
	}
}

bool ATotemPawn::ServerEquipWeapon_Validate(ABaseWeapon* Weapon) { return true; }

void ATotemPawn::ServerEquipWeapon_Implementation(ABaseWeapon* Weapon)
{
	EquipWeapon(Weapon);
}

void ATotemPawn::OnRep_CurrentWeapon(ABaseWeapon* LastWeapon)
{
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
}

void ATotemPawn::OnRep_Inventory()
{
	OnUpdateInventory();
}

void ATotemPawn::OnNextWeapon()
{
	if (Inventory.Num() >= 2)
	{
		ABaseWeapon* NextWeapon = Inventory[(GetCurrentWeaponIndex() + 1) % Inventory.Num()];
		EquipWeapon(NextWeapon);
	}
}

void ATotemPawn::OnPrevWeapon()
{
	if (Inventory.Num() >= 2)
	{
		ABaseWeapon* PrevWeapon = Inventory[(GetCurrentWeaponIndex() - 1 + Inventory.Num()) % Inventory.Num()];
		EquipWeapon(PrevWeapon);
	}
}

void ATotemPawn::TryEquipWeaponByIndex(int32 Index)
{
	if (Index < Inventory.Num())
	{
		if (Index != GetCurrentWeaponIndex())
		{
			EquipWeapon(Inventory[Index]);
		}
	}
}

int32 ATotemPawn::GetCurrentWeaponIndex()
{
	int32 Index = 0;
	if (CurrentWeapon)
	{
		Index = Inventory.IndexOfByKey(CurrentWeapon);
	}
	return Index;
}

void ATotemPawn::OnUpdateInventory_Implementation() {}

void ATotemPawn::GroundEffects_Implementation(FVector GroundPoint) {}

void ATotemPawn::ReplicateHit(float Damage, FDamageEvent const& DamageEvent, APawn* InstigatingPawn, AActor* DamageCauser, bool bKilled)
{
	const float TimeoutTime = GetWorld()->GetTimeSeconds() + 0.5f;

	FDamageEvent const& LastDamageEvent = LastTakeHitInfo.GetDamageEvent();
	if ((InstigatingPawn == LastTakeHitInfo.PawnInstigator.Get()) && (LastDamageEvent.DamageTypeClass == LastTakeHitInfo.DamageTypeClass) && (LastTakeHitTimeTimeout == TimeoutTime))
	{
		if (bKilled && LastTakeHitInfo.bKilled)
		{
			return;
		}
		Damage += LastTakeHitInfo.ActualDamage;
	}

	LastTakeHitInfo.ActualDamage = Damage;
	LastTakeHitInfo.PawnInstigator = Cast<ATotemPawn>(InstigatingPawn);
	LastTakeHitInfo.DamageCauser = DamageCauser;
	LastTakeHitInfo.SetDamageEvent(DamageEvent);
	LastTakeHitInfo.bKilled = bKilled;
	LastTakeHitInfo.EnsureReplication();

	LastTakeHitTimeTimeout = TimeoutTime;
}

void ATotemPawn::OnRep_LastTakeHitInfo()
{
	if (LastTakeHitInfo.bKilled)
	{
		if (!IsDead)
		{
			IsDead = true;
			OnDeath(LastTakeHitInfo.ActualDamage, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.PawnInstigator.Get(), LastTakeHitInfo.DamageCauser.Get());
		}
	}
}

void ATotemPawn::SetCurrentHealth(float HealthValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(HealthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}

void ATotemPawn::UpdateHealth_Implementation(float NewHealth) {}

float ATotemPawn::TakeDamage(float DamageTaken, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	ATotemGameMode* GameMode = Cast<ATotemGameMode>(GetWorld()->GetAuthGameMode());

	if (GameMode)
	{
		if (GameMode->CanDamage(EventInstigator, GetController()))
		{
			float DamageApplied = CurrentHealth - DamageTaken;
			SetCurrentHealth(DamageApplied);

			IsRegenerating = false;

			if (CurrentHealth <= 0)
			{
				Die(DamageTaken, DamageEvent, EventInstigator, DamageCauser);
			}
			else
			{
				GetWorldTimerManager().SetTimer(RegenerationTimer, this, &ATotemPawn::BeginRegeneration, RegenerationDelay, false);
			}

			MakeNoise(1.0f, EventInstigator ? EventInstigator->GetPawn() : this);
			return DamageApplied;
		}
	}

	return 0.f;
}

void ATotemPawn::BeginRegeneration()
{
	if (!IsDead)
	{
		IsRegenerating = true;
	}
}

void ATotemPawn::Enable() { IsEnabled = true; }
void ATotemPawn::Disable() { IsEnabled = false; }
void ATotemPawn::SetPaused(bool Paused) { IsPaused = Paused; }

void ATotemPawn::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->ActorHasTag("Exp"))
	{
		GetWorldTimerManager().SetTimer(ExplosionTimer, this, &ATotemPawn::EndExplosionEffect, ExplosionEffectTime, false);
		DistanceFromExplosion = FVector::Distance(OtherActor->GetActorLocation(), GetActorLocation());
	}
}

void ATotemPawn::EndExplosionEffect() {}

float ATotemPawn::HorizontalSpeed()
{
	FVector Velocity = TotemMesh->GetPhysicsLinearVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ATotemPawn::Destroyed()
{
	Super::Destroyed();
	DestroyInventory();
}

void ATotemPawn::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = GetMaxHealth();
		SpawnDefaultInventory();
	}

	// Create dissolve material instance
	if (DissolveMaterial && TotemMesh)
	{
		DissolveMID = TotemMesh->CreateDynamicMaterialInstance(0, DissolveMaterial);
	}

	// Start ambient loop
	if (AmbientLoop && AmbientLoop->Sound)
	{
		AmbientLoop->Play();
	}

	// Set angular damping
	if (TotemMesh)
	{
		TotemMesh->SetAngularDamping(AngularDamping);
	}

	// Set up dissolve timeline
	if (DissolveCurve)
	{
		FOnTimelineFloat DissolveUpdate;
		DissolveUpdate.BindUFunction(this, FName("DissolveTimelineUpdate"));
		FOnTimelineEvent DissolveFinished;
		DissolveFinished.BindUFunction(this, FName("DissolveTimelineFinished"));
		DissolveTimeline.AddInterpFloat(DissolveCurve, DissolveUpdate);
		DissolveTimeline.SetTimelineFinishedFunc(DissolveFinished);
		DissolveTimeline.SetPropertySetObject(this);
	}

	// Set up nametag fade timeline
	if (FadeNametagCurve)
	{
		FOnTimelineFloat NametagUpdate;
		NametagUpdate.BindUFunction(this, FName("FadeNametagUpdate"));
		FOnTimelineEvent NametagFinished;
		NametagFinished.BindUFunction(this, FName("FadeNametagFinished"));
		FadeNametagTimeline.AddInterpFloat(FadeNametagCurve, NametagUpdate);
		FadeNametagTimeline.SetTimelineFinishedFunc(NametagFinished);
	}
}

void ATotemPawn::SpawnDefaultInventory()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	int32 NumWeaponClasses = DefaultInventoryClasses.Num();
	for (int32 i = 0; i < NumWeaponClasses; i++)
	{
		if (DefaultInventoryClasses[i])
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ABaseWeapon* NewWeapon = GetWorld()->SpawnActor<ABaseWeapon>(DefaultInventoryClasses[i], SpawnInfo);
			AddWeapon(NewWeapon);
		}
	}

	if (Inventory.Num() > 0)
	{
		EquipWeapon(Inventory[0]);
	}
}

ABaseWeapon* ATotemPawn::GiveWeapon(TSubclassOf<class ABaseWeapon> WeaponClass)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.Owner = this;
	ABaseWeapon* NewWeapon = GetWorld()->SpawnActor<ABaseWeapon>(WeaponClass, SpawnInfo);
	AddWeapon(NewWeapon);
	return NewWeapon;
}

AShooterWeapon* ATotemPawn::GiveShooterWeapon(TSubclassOf<class AShooterWeapon> WeaponClass, int Ammo)
{
	AShooterWeapon* NewWeapon = Cast<AShooterWeapon>(GiveWeapon(WeaponClass));
	NewWeapon->GiveAmmo(Ammo);
	return NewWeapon;
}

void ATotemPawn::PostInitProperties()
{
	Super::PostInitProperties();
}

void ATotemPawn::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
	DOREPLIFETIME_ACTIVE_OVERRIDE(ATotemPawn, LastTakeHitInfo, GetWorld() && GetWorld()->GetTimeSeconds() < LastTakeHitTimeTimeout);
}

void ATotemPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ATotemPawn, Inventory, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ATotemPawn, LastTakeHitInfo, COND_Custom);
	DOREPLIFETIME(ATotemPawn, CurrentWeapon);
	DOREPLIFETIME(ATotemPawn, CurrentHealth);
	DOREPLIFETIME(ATotemPawn, Team);
	DOREPLIFETIME(ATotemPawn, bDashEffectsPending);
}

void ATotemPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Tick timelines
	if (DissolveTimeline.IsPlaying() || DissolveTimeline.IsReversing())
	{
		DissolveTimeline.TickTimeline(DeltaSeconds);
	}
	if (FadeNametagTimeline.IsPlaying() || FadeNametagTimeline.IsReversing())
	{
		FadeNametagTimeline.TickTimeline(DeltaSeconds);
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		if (GetActorLocation().Z < KillZ)
		{
			Die(0.f, FDamageEvent(), NULL, NULL);
		}

		if (IsRegenerating && CurrentHealth < MaxHealth && !IsDead)
		{
			SetCurrentHealth(GetCurrentHealth() + RegenerationRate * DeltaSeconds);
		}
	}

	if (!IsLocallyControlled())
	{
		return;
	}

	if (IsEnabled && !IsDead)
	{
		ApplyHoverForce(DeltaSeconds);
		ApplyMovementForce(DeltaSeconds);
		ApplyStabilizationTorque(DeltaSeconds);
	}

	if (IsDashing)
	{
		// Dash rotation interpolation driven by curve
		if (DashCurve)
		{
			float DashProgress = 1.0f - (GetWorldTimerManager().GetTimerElapsed(DashTimer) / DashDuration);
			DashProgress = FMath::Clamp(DashProgress, 0.f, 1.f);
			float CurveValue = DashCurve->GetFloatValue(DashProgress);

			if (DashRotationAxis == EDashRotationAxis::Roll || DashRotationAxis == EDashRotationAxis::Both)
			{
				CurrentDashRoll = CurveValue * CurrentSideDashDirection * 45.0f;
			}
			if (DashRotationAxis == EDashRotationAxis::Pitch || DashRotationAxis == EDashRotationAxis::Both)
			{
				CurrentDashPitch = CurveValue * CurrentForwardDashDirection * 30.0f;
			}
		}
	}

	UpdateCameraState(DeltaSeconds);

	if (IsEnabled && !IsDead)
	{
		UpdateWindVolume(DeltaSeconds);
	}

	// Camera rotation
	FRotator ControlRotation = GetControlRotation();
	if (!IsPaused)
	{
		SpringArm->SetRelativeRotation(ControlRotation);
	}

	// Control firing
	if (!bIsFiring && bWantsToFire && !IsPaused && !IsDashing)
	{
		if (CurrentWeapon && CurrentWeapon->bIsEquipped)
		{
			CurrentWeapon->StartFire();
			bIsFiring = true;
		}
	}
	if (bIsFiring && (!bWantsToFire || IsDashing))
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->StopFire();
			bIsFiring = false;
		}
	}

	if (!bIsFiringAlt && bWantsToFireAlt && !IsPaused && !IsDashing)
	{
		if (CurrentWeapon && CurrentWeapon->bIsEquipped)
		{
			CurrentWeapon->StartFireAlt();
			bIsFiringAlt = true;
		}
	}
	if (bIsFiringAlt && (!bWantsToFireAlt || IsDashing))
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->StopFireAlt();
			bIsFiringAlt = false;
		}
	}

	UpdateNametag(DeltaSeconds);
}

void ATotemPawn::ApplyHoverForce(float DeltaSeconds)
{
	if (!TotemMesh)
	{
		return;
	}

	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0.f, 0.f, MaxHoverHeight);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, HoverTraceChannel, QueryParams);

	if (bHit)
	{
		IsGrounded = true;
		float MeasuredHeight = HitResult.Distance;

		// Feed error into PID
		if (HoverPID)
		{
			float Error = DesiredHoverHeight - MeasuredHeight;
			HoverPID->AddError(Error, DeltaSeconds);
			float ControlOutput = HoverPID->GetControlCoefficient();

			// Apply upward force scaled by PID output
			float ForceMagnitude = FMath::Clamp(ControlOutput * HoverForce, 0.f, HoverForce * 2.f);
			FVector UpForce = FVector(0.f, 0.f, ForceMagnitude);
			TotemMesh->AddForceAtLocation(UpForce, ForceApplication->GetComponentLocation());

			// Ground effects
			GroundEffects(HitResult.ImpactPoint);
		}
	}
	else
	{
		IsGrounded = false;
	}
}

void ATotemPawn::ApplyMovementForce(float DeltaSeconds)
{
	if (!TotemMesh)
	{
		return;
	}

	// Consume accumulated movement input
	FVector InputVector = ConsumeMovementInputVector();
	if (!InputVector.IsNearlyZero())
	{
		float Speed = HorizontalSpeed();
		float AccelerationFactor = 1.0f;
		if (AccelerationCurve)
		{
			AccelerationFactor = AccelerationCurve->GetFloatValue(Speed);
		}

		// Transform input to world space using control rotation
		FRotator ControlRot = GetControlRotation();
		FVector Forward = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::X);
		FVector Right = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::Y);
		Forward.Z = 0.f;
		Forward.Normalize();
		Right.Z = 0.f;
		Right.Normalize();

		FVector WorldForce = (Forward * MoveY + Right * MoveX) * MovementForce * AccelerationFactor;
		TotemMesh->AddForceAtLocation(WorldForce, ForceApplication->GetComponentLocation());
	}
}

void ATotemPawn::ApplyStabilizationTorque(float DeltaSeconds)
{
	if (!TotemMesh)
	{
		return;
	}

	FRotator CurrentRot = GetActorRotation();

	// Desired rotation is control rotation flattened (no pitch/roll) plus dash offset
	FRotator ControlRot = GetControlRotation();
	FRotator TargetRot = FRotator(0.f, ControlRot.Yaw, 0.f);

	// Apply dash roll/pitch offset
	if (IsDashing)
	{
		TargetRot.Roll = CurrentDashRoll;
		TargetRot.Pitch = CurrentDashPitch;
	}

	// Compute torque to align to target rotation
	FRotator Error = TargetRot - CurrentRot;
	Error.Normalize();

	FVector Torque;
	Torque.X = Error.Pitch * PitchStabilization * DeltaSeconds;
	Torque.Y = Error.Roll * RollStabilization * DeltaSeconds;
	Torque.Z = Error.Yaw * YawStabilization * DeltaSeconds;

	TotemMesh->AddTorqueInRadians(Torque);

	// Angular damping
	TotemMesh->SetAngularDamping(AngularDamping);
}

void ATotemPawn::UpdateCameraState(float DeltaSeconds)
{
	float DesiredDistance = IsAiming ? AimingCameraDistance : NormalCameraDistance;
	float DesiredFOV = IsAiming ? AimingCameraFOV : NormalCameraFOV;

	if (SpringArm)
	{
		SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, DesiredDistance, DeltaSeconds, CameraDistanceLerp);
	}

	if (Camera)
	{
		float CurrentFOV = Camera->FieldOfView;
		Camera->SetFieldOfView(FMath::FInterpTo(CurrentFOV, DesiredFOV, DeltaSeconds, CameraFOVLerp));
	}
}

void ATotemPawn::UpdateWindVolume(float DeltaSeconds)
{
	if (!AmbientLoop)
	{
		return;
	}

	float SpeedSq = HorizontalSpeed() * HorizontalSpeed();
	float WindVolume = FMath::Clamp(SpeedSq / SpeedSquaredAtMaxWindVolume, 0.f, 1.f);
	AmbientLoop->SetVolumeMultiplier(WindVolume);
}

void ATotemPawn::UpdateNametag(float DeltaSeconds)
{
	// Nametag visibility based on whether this is the locally controlled pawn
	if (Nametag)
	{
		bool bShouldShow = !IsLocallyControlled();
		if (bShouldShow != bNametagVisible)
		{
			bNametagVisible = bShouldShow;
			Nametag->SetVisibility(bShouldShow);
		}
	}
}

void ATotemPawn::BeginDash()
{
	if (IsDashing || !WantsToDash || IsDead || !IsEnabled)
	{
		return;
	}

	// Stop firing during dash
	if (bIsFiring && CurrentWeapon)
	{
		CurrentWeapon->StopFire();
		bIsFiring = false;
	}
	if (bIsFiringAlt && CurrentWeapon)
	{
		CurrentWeapon->StopFireAlt();
		bIsFiringAlt = false;
	}

	IsDashing = true;

	// Determine dash direction from input
	float ForwardDir = MoveY;
	float RightDir = MoveX;
	CurrentForwardDashDirection = FMath::Sign(ForwardDir);
	CurrentSideDashDirection = FMath::Sign(RightDir);

	// If no input, dash forward
	if (ForwardDir == 0.f && RightDir == 0.f)
	{
		ForwardDir = 1.0f;
		CurrentForwardDashDirection = 1;
	}

	// Build dash impulse from control rotation
	FRotator ControlRot = GetControlRotation();
	FVector Forward = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::X);
	FVector Right = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::Y);
	FVector Up = FVector(0.f, 0.f, 1.f);
	Forward.Z = 0.f;
	Forward.Normalize();
	Right.Z = 0.f;
	Right.Normalize();

	FVector DashImpulse = (Forward * ForwardDir * DashForwardAxisMultiplier)
		+ (Right * RightDir * DashRightAxisMultiplier)
		+ (Up * DashUpAxisMultiplier);
	DashImpulse *= DashForce;

	TotemMesh->AddImpulseAtLocation(DashImpulse, DashForcePoint->GetComponentLocation());

	// Spawn dash effects locally
	SpawnDashEffects();

	// Mark for replication
	if (GetLocalRole() == ROLE_Authority)
	{
		bDashEffectsPending = true;
	}

	// Set dash completion timer
	GetWorldTimerManager().SetTimer(DashTimer, this, &ATotemPawn::StopDash, DashDuration, false);
}

void ATotemPawn::StopDash()
{
	IsDashing = false;
	CurrentDashRoll = 0.f;
	CurrentDashPitch = 0.f;
	WantsToDash = false;

	if (GetLocalRole() == ROLE_Authority)
	{
		bDashEffectsPending = false;
	}

	RechargeDash();
}

void ATotemPawn::RechargeDash_Implementation() {}

void ATotemPawn::SpawnDashEffects_Implementation()
{
	if (DashEffects && TotemMesh)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(DashEffects, TotemMesh, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
	}
	if (DashTrails && TotemMesh)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(DashTrails, TotemMesh, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
	}
}

void ATotemPawn::OnRep_DashEffects()
{
	if (bDashEffectsPending)
	{
		SpawnDashEffects();
	}
}

void ATotemPawn::DestroyInventory()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	for (int32 i = Inventory.Num() - 1; i >= 0; i--)
	{
		ABaseWeapon* Weapon = Inventory[i];
		if (Weapon)
		{
			RemoveWeapon(Weapon);
			Weapon->Destroy();
		}
	}
}

float ATotemPawn::GetScopeZoom()
{
	return CurrentWeapon ? CurrentWeapon->ScopeZoom : 1.0f;
}

void ATotemPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	// Movement controls
	PlayerInputComponent->BindAxis("Horizontal", this, &ATotemPawn::HorizontalInput);
	PlayerInputComponent->BindAxis("Vertical", this, &ATotemPawn::VerticalInput);

	// Look controls
	PlayerInputComponent->BindAxis("Turn", this, &ATotemPawn::AddYaw);
	PlayerInputComponent->BindAxis("LookUp", this, &ATotemPawn::AddPitch);

	// Firing controls
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ATotemPawn::FirePressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ATotemPawn::FireReleased);
	PlayerInputComponent->BindAction("FireAlt", IE_Pressed, this, &ATotemPawn::FireAltPressed);
	PlayerInputComponent->BindAction("FireAlt", IE_Released, this, &ATotemPawn::FireAltReleased);

	// Dash
	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &ATotemPawn::DashPressed);
	PlayerInputComponent->BindAction("Dash", IE_Released, this, &ATotemPawn::DashReleased);

	// Aim
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ATotemPawn::AimPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ATotemPawn::AimReleased);

	// Interact
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ATotemPawn::InteractPressed);

	// Weapon switching
	PlayerInputComponent->BindAction("Weapon1", IE_Pressed, this, &ATotemPawn::Weapon1);
	PlayerInputComponent->BindAction("Weapon2", IE_Pressed, this, &ATotemPawn::Weapon2);
	PlayerInputComponent->BindAction("Weapon3", IE_Pressed, this, &ATotemPawn::Weapon3);
	PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &ATotemPawn::OnNextWeapon);
	PlayerInputComponent->BindAction("PrevWeapon", IE_Pressed, this, &ATotemPawn::OnPrevWeapon);
}

bool ATotemPawn::Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return false;
	}

	if (IsDead)
	{
		return false;
	}

	IsDead = true;
	CurrentHealth = FMath::Min(0.0f, CurrentHealth);

	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	Killer = GetDamageInstigator(Killer, *DamageType);

	AController* KilledPlayer = Controller != nullptr ? Controller.Get() : Cast<AController>(GetOwner());
	if (GetWorld()->GetAuthGameMode<ATotemGameMode>())
	{
		GetWorld()->GetAuthGameMode<ATotemGameMode>()->Killed(Killer, KilledPlayer, this, DamageType);
	}

	APawn* InstigatingPawn = Killer ? Killer->GetPawn() : NULL;

	ReplicateHit(KillingDamage, DamageEvent, InstigatingPawn, DamageCauser, true);
	OnDeath(KillingDamage, DamageEvent, InstigatingPawn, DamageCauser);

	ATotemPlayerController* PlayerController = Cast<ATotemPlayerController>(GetController());
	if (PlayerController)
	{
		PlayerController->OnDeath();
	}

	return true;
}

void ATotemPawn::OnDeath_Implementation(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser)
{
	// Play death sound for self
	if (YouDiedSoundCue)
	{
		UGameplayStatics::PlaySound2D(this, YouDiedSoundCue);
	}

	// Start dissolve out
	DissolveOut();

	// Freeze after dissolve completes (handled by DissolveTimelineFinished)
	Disable();
}

void ATotemPawn::Freeze()
{
	bFrozen = true;
	if (TotemMesh)
	{
		TotemMesh->SetSimulatePhysics(false);
		TotemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (AmbientLoop)
	{
		AmbientLoop->Stop();
	}
}

void ATotemPawn::DissolveOut()
{
	if (!DissolveTimeline.IsPlaying() && DissolveMID)
	{
		DissolveTimeline.PlayFromStart();
	}
}

void ATotemPawn::DissolveIn()
{
	if (DissolveMID)
	{
		DissolveTimeline.ReverseFromEnd();
	}
}

void ATotemPawn::DissolveTimelineUpdate(float Value)
{
	if (DissolveMID)
	{
		DissolveMID->SetScalarParameterValue(TEXT("DissolveAmount"), Value);
	}
}

void ATotemPawn::DissolveTimelineFinished()
{
	if (DissolveTimeline.GetPlaybackPosition() >= DissolveTimeline.GetTimelineLength() && !DissolveTimeline.IsReversing())
	{
		Freeze();
	}
	OnDissolveFinished.Broadcast();
}

void ATotemPawn::ReceivePossessed_Implementation(AController* NewController)
{
	// Re-enable on respawn
	if (IsDead)
	{
		IsDead = false;
		bFrozen = false;
		IsEnabled = true;
		CurrentHealth = MaxHealth;

		if (TotemMesh)
		{
			TotemMesh->SetSimulatePhysics(true);
			TotemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}

		DissolveIn();

		if (AmbientLoop && AmbientLoop->Sound)
		{
			AmbientLoop->Play();
		}
	}
}

void ATotemPawn::HorizontalInput(float Val)
{
	FVector Right = FRotationMatrix(GetControlRotation()).GetScaledAxis(EAxis::Y);
	AddMovementInput(Right * Val);
	MoveX = Val;
}

void ATotemPawn::VerticalInput(float Val)
{
	FVector Forward = FRotationMatrix(GetControlRotation()).GetScaledAxis(EAxis::X);
	Forward.Z = 0.f;
	Forward.Normalize();

	AddMovementInput(Forward * Val);
	MoveY = Val;
}

void ATotemPawn::AddYaw(float Val)
{
	if (!IsPaused)
	{
		AddControllerYawInput(Val);
	}
}

void ATotemPawn::AddPitch(float Val)
{
	if (!IsPaused)
	{
		AddControllerPitchInput(Val);
	}
}

void ATotemPawn::FirePressed() { bWantsToFire = true; }
void ATotemPawn::FireReleased() { bWantsToFire = false; }
void ATotemPawn::FireAltPressed() { bWantsToFireAlt = true; }
void ATotemPawn::FireAltReleased() { bWantsToFireAlt = false; }

void ATotemPawn::DashPressed()
{
	WantsToDash = true;
	BeginDash();
}

void ATotemPawn::DashReleased()
{
	WantsToDash = false;
}

void ATotemPawn::AimPressed()
{
	ShouldAim = true;
	IsAiming = true;
}

void ATotemPawn::AimReleased()
{
	ShouldAim = false;
	IsAiming = false;
}

void ATotemPawn::InteractPressed()
{
	CheckInteract();
}

void ATotemPawn::CheckInteract()
{
	if (!IsEnabled || IsDead)
	{
		return;
	}

	FVector Start = Camera ? Camera->GetComponentLocation() : GetActorLocation();
	FVector Forward = Camera ? Camera->GetForwardVector() : GetActorForwardVector();
	FVector End = Start + Forward * MaxInteractDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_GameTraceChannel6, QueryParams))
	{
		if (HitResult.GetActor())
		{
			IsInteracting = true;
			OnInteract(HitResult.GetActor());
		}
	}
	else
	{
		IsInteracting = false;
	}
}

void ATotemPawn::OnInteract_Implementation(AActor* Interactable) {}

void ATotemPawn::Weapon1() { TryEquipWeaponByIndex(0); }
void ATotemPawn::Weapon2() { TryEquipWeaponByIndex(1); }
void ATotemPawn::Weapon3() { TryEquipWeaponByIndex(2); }

void ATotemPawn::FireCurrentWeapon()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void ATotemPawn::OnHealthUpdate()
{
	UpdateHealth(CurrentHealth);
}

void ATotemPawn::SetName(const FString& NewName)
{
	if (Nametag)
	{
		Nametag->SetText(FText::FromString(NewName));
	}
}

void ATotemPawn::SetAccentColor(FLinearColor Color)
{
	if (TotemMesh)
	{
		UMaterialInstanceDynamic* DynMat = TotemMesh->CreateDynamicMaterialInstance(0);
		if (DynMat)
		{
			DynMat->SetVectorParameterValue(TEXT("AccentColor"), Color);
		}
	}
	if (TotemOutline)
	{
		UMaterialInstanceDynamic* OutlineMat = TotemOutline->CreateDynamicMaterialInstance(0);
		if (OutlineMat)
		{
			OutlineMat->SetVectorParameterValue(TEXT("AccentColor"), Color);
			TotemOutline->SetVisibility(true);
		}
	}
}

void ATotemPawn::SetOutlineVisibility(bool bVisible)
{
	if (TotemOutline)
	{
		TotemOutline->SetVisibility(bVisible);
	}
}

void ATotemPawn::ShouldUpdateNametag_Implementation() {}

void ATotemPawn::FadeNametagUpdate(float Value)
{
	if (Nametag)
	{
		FColor CurrentColor = Nametag->TextRenderColor;
		CurrentColor.A = static_cast<uint8>(Value * 255);
		Nametag->SetTextRenderColor(CurrentColor);
	}
}

void ATotemPawn::FadeNametagFinished() {}

void ATotemPawn::SetTeam(int32 NewTeam)
{
	Team = NewTeam;
}

void ATotemPawn::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
}
