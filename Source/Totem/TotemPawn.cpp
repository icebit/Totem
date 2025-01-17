// Copyright Epic Games, Inc. All Rights Reserved.

#include "TotemPawn.h"
#include "TotemGameMode.h"
#include "TakeHitInfo.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "BaseWeapon.h"
#include "ShooterWeapon.h"
#include "Engine/StaticMesh.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/KismetMathLibrary.h"
#include <Runtime\Engine\Classes\Kismet\GameplayStatics.h>
#include <Runtime\Engine\Public\Net\UnrealNetwork.h>
#include "PIDComponent.h"
#include <Totem\TotemPlayerController.h>

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

	WeaponMountPoint = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponMountPoint0"));
	WeaponMountPoint->SetupAttachment(TotemMesh);
	WeaponMountPoint->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(TotemMesh);
	SpringArm->TargetArmLength = TargetCameraDistance;
	SpringArm->bEnableCameraLag = false;

	// Disable inherit rotation
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;

	// Create camera component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);	// Attach the camera
	Camera->bUsePawnControlRotation = false;

	NeutralHoverPhase = FMath::RandRange(0.f, 6.28f);
}

void ATotemPawn::SetTargetCameraDistance(float Distance)
{
	TargetCameraDistance = Distance;
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

	// unequip previous
	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnequip();
		bIsFiring = false;
		bIsFiringAlt = false;
	}

	CurrentWeapon = NewWeapon;

	// equip new one
	if (NewWeapon)
	{
		NewWeapon->SetMyPawn(this);	// Make sure weapon's MyPawn is pointing back to us. During replication, we can't guarantee APawn::CurrentWeapon will rep after AWeapon::MyPawn!

		NewWeapon->OnEquip(LastWeapon);
	}
}

bool ATotemPawn::ServerEquipWeapon_Validate(ABaseWeapon* Weapon)
{
	return true;
}

void ATotemPawn::ServerEquipWeapon_Implementation(ABaseWeapon* Weapon)
{
	EquipWeapon(Weapon);
}

/*
bool ATotemPawn::ServerDash_Validate(float SideDash, float ForwardDash)
{
	return true;
}

void ATotemPawn::ServerDash_Implementation(float SideDash, float ForwardDash)
{
	if(bIsFiring)
	{
		CurrentWeapon->StopFire();	
	}
	bIsFiring = false;
	if(bIsFiringAlt)
	{
		CurrentWeapon->StopFireAlt();
	}
	bIsFiringAlt = false;

	//CurrentWeapon->OnUnequip();
	
	BeginDashEffects(SideDash, ForwardDash);

	GetWorldTimerManager().SetTimer(DashTimer, this, &ATotemPawn::RechargeDash, DashTime, false);
	//GetWorld()->GetTimerManager().SetTimer(DashReequipTimer, this, &ATotemPawn::ReequipDashWeapon, DashWeaponReequipTime, false);

	ClientDash(SideDash, ForwardDash);
}

void ATotemPawn::ClientDash_Implementation(float SideDash, float ForwardDash)
{
	if(GetLocalRole() < ROLE_AutonomousProxy)
	{
		//CurrentWeapon->OnUnequip();
	
		BeginDashEffects(SideDash, ForwardDash);

		GetWorldTimerManager().SetTimer(DashTimer, this, &ATotemPawn::RechargeDash, DashTime, false);
		//GetWorld()->GetTimerManager().SetTimer(DashReequipTimer, this, &ATotemPawn::ReequipDashWeapon, DashWeaponReequipTime, false);
	}
}*/

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

void ATotemPawn::TryEquipWeaponByIndex(int32 Index) {
	if (Index < Inventory.Num()) {
		if (Index != GetCurrentWeaponIndex()) {
			EquipWeapon(Inventory[Index]);
		}
	}
}

int32 ATotemPawn::GetCurrentWeaponIndex() {
	int32 Index = 0;
	
	if (CurrentWeapon) {
		Index = Inventory.IndexOfByKey(CurrentWeapon);
	}

	return Index;
}

void ATotemPawn::OnUpdateInventory_Implementation()
{
}

void ATotemPawn::GroundEffects_Implementation(FVector GroundPoint)
{
}

void ATotemPawn::ReplicateHit(float Damage, FDamageEvent const& DamageEvent, APawn* InstigatingPawn, AActor* DamageCauser, bool bKilled)
{
	const float TimeoutTime = GetWorld()->GetTimeSeconds() + 0.5f;

	FDamageEvent const& LastDamageEvent = LastTakeHitInfo.GetDamageEvent();
	if ((InstigatingPawn == LastTakeHitInfo.PawnInstigator.Get()) && (LastDamageEvent.DamageTypeClass == LastTakeHitInfo.DamageTypeClass) && (LastTakeHitTimeTimeout == TimeoutTime))
	{
		// same frame damage
		if (bKilled && LastTakeHitInfo.bKilled)
		{
			// Redundant death take hit, just ignore it
			return;
		}

		// otherwise, accumulate damage done this frame
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
		if (!IsDead) {
			IsDead = true;
			OnDeath(LastTakeHitInfo.ActualDamage, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.PawnInstigator.Get(), LastTakeHitInfo.DamageCauser.Get());
		}
	}
	else
	{
		//PlayHit(LastTakeHitInfo.ActualDamage, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.PawnInstigator.Get(), LastTakeHitInfo.DamageCauser.Get());
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

void ATotemPawn::UpdateHealth_Implementation(float NewHealth)
{
}

float ATotemPawn::TakeDamage(float DamageTaken, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	ATotemGameMode* GameMode = Cast<ATotemGameMode>(GetWorld()->GetAuthGameMode());

	if (GameMode) {
		// Only apply damage if is allowed
		if (GameMode->CanDamage(EventInstigator, GetController())) {
			float DamageApplied = CurrentHealth - DamageTaken;
			SetCurrentHealth(DamageApplied);

			// Delay regeneration
			IsRegenerating = false;

			if (CurrentHealth <= 0)
			{
				Die(DamageTaken, DamageEvent, EventInstigator, DamageCauser);
			}
			else {
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
	if(!IsDead)
	{
		IsRegenerating = true;		
	}
}

void ATotemPawn::Enable()
{
	IsEnabled = true;
}

void ATotemPawn::Disable()
{
	IsEnabled = false;
}

void ATotemPawn::SetPaused(bool Paused)
{
	IsPaused = Paused;
}

// Reduce movement force when explosion is nearby
// Maybe replace this by monitoring acceleration?
void ATotemPawn::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Exp")) {
		GetWorldTimerManager().SetTimer(ExplosionTimer, this, &ATotemPawn::EndExplosionEffect, ExplosionEffectTime, false);
		DistanceFromExplosion = FVector::Distance(OtherActor->GetActorLocation(), GetActorLocation());
	}
}

void ATotemPawn::EndExplosionEffect()
{
}

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

	// Spawn default inventory
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = GetMaxHealth();
		SpawnDefaultInventory();
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

	// Equip first weapon in inventory
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
	//NewWeapon->SetOwner(GetController());
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

	// Only replicate this property for a short duration after it changes so join in progress players don't get spammed with fx when joining late
	DOREPLIFETIME_ACTIVE_OVERRIDE(ATotemPawn, LastTakeHitInfo, GetWorld() && GetWorld()->GetTimeSeconds() < LastTakeHitTimeTimeout);
}

void ATotemPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ATotemPawn, Inventory, COND_OwnerOnly);

	DOREPLIFETIME_CONDITION(ATotemPawn, LastTakeHitInfo, COND_Custom);

	DOREPLIFETIME(ATotemPawn, CurrentWeapon);
	DOREPLIFETIME(ATotemPawn, CurrentHealth);
}

void ATotemPawn::Tick(float DeltaSeconds)
{
	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);

	if (GetLocalRole() == ROLE_Authority) {
		if (GetActorLocation().Z < KillZ) {
			Die(0.f, FDamageEvent(), NULL, NULL);
		}

		if (IsRegenerating && CurrentHealth < MaxHealth && !IsDead) {
			SetCurrentHealth(GetCurrentHealth() + RegenerationRate * DeltaSeconds);
		}
	}

	// Don't run anything unless locally controlled
	if (!IsLocallyControlled()) {
		return;
	}

	// Interpolate camera distance
	if (SpringArm) {
		SpringArm->TargetArmLength = FMath::Lerp(SpringArm->TargetArmLength, TargetCameraDistance, CameraDistanceLerp * DeltaSeconds);
	}
	
	// TODO: Set hover component position?

	// Set camera rotation
	FRotator ControlRotation = GetControlRotation();
	if (!IsPaused) {
		SpringArm->SetRelativeRotation(ControlRotation);
	}
	
	// Control firing	
	if(!bIsFiring && bWantsToFire && !IsPaused)
	{
		if (CurrentWeapon && CurrentWeapon->bIsEquipped)
		{
			CurrentWeapon->StartFire();
			bIsFiring = true;
		}
	}
	if(bIsFiring && !bWantsToFire)
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->StopFire();
			bIsFiring = false;
		}
	}
	
	if(!bIsFiringAlt && bWantsToFireAlt && !IsPaused)
	{
		if (CurrentWeapon && CurrentWeapon->bIsEquipped)
		{
			CurrentWeapon->StartFireAlt();
			bIsFiringAlt = true;
		}
	}
	if(bIsFiringAlt && !bWantsToFireAlt)
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->StopFireAlt();
			bIsFiringAlt = false;
		}
	}
}

void ATotemPawn::DestroyInventory()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	// remove all weapons from inventory and destroy them
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
	return CurrentWeapon->ScopeZoom;
}

void ATotemPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
		// Check if PlayerInputComponent is valid (not NULL)
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

	// Weapon switching
	PlayerInputComponent->BindAction("Weapon1", IE_Pressed, this, &ATotemPawn::Weapon1);
	PlayerInputComponent->BindAction("Weapon2", IE_Pressed, this, &ATotemPawn::Weapon2);
	PlayerInputComponent->BindAction("Weapon3", IE_Pressed, this, &ATotemPawn::Weapon3);
	PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &ATotemPawn::OnNextWeapon);
	PlayerInputComponent->BindAction("PrevWeapon", IE_Pressed, this, &ATotemPawn::OnPrevWeapon);
}

bool ATotemPawn::Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser)
{	// Only run this function on authority
	if (GetLocalRole() != ROLE_Authority)
	{
		return false;
	}

	if (IsDead) {
		return false;
	}

	IsDead = true;

	CurrentHealth = FMath::Min(0.0f, CurrentHealth);

	// if this is an environmental death then refer to the previous killer so that they receive credit (knocked into lava pits, etc)
	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	Killer = GetDamageInstigator(Killer, *DamageType);

	//AController* KilledPlayer = Controller != nullptr ? Controller.Get() : Cast<AController>(GetOwner());
	//GetWorld()->GetAuthGameMode<ATotemGameMode>()->Killed(Killer, KilledPlayer, this, DamageType);

	APawn* InstigatingPawn = Killer ? Killer->GetPawn() : NULL;

	// Replicate
	ReplicateHit(KillingDamage, DamageEvent, InstigatingPawn, DamageCauser, true);

	// Call OnDeath event in blueprint to disable totem and play effects
	OnDeath(KillingDamage, DamageEvent, InstigatingPawn, DamageCauser);
	
	ATotemPlayerController* PlayerController = Cast<ATotemPlayerController>(GetController());
	if (PlayerController) {
		PlayerController->OnDeath();
	}

	return true;
}

void ATotemPawn::OnDeath_Implementation(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser)
{
}

void ATotemPawn::HorizontalInput(float Val)
{
	// Get right vector
	FVector Right = FRotationMatrix(GetControlRotation()).GetScaledAxis(EAxis::Y);

	AddMovementInput(Right * Val);
	//MoveY = Val;
}

void ATotemPawn::VerticalInput(float Val)
{
	FVector Forward = FRotationMatrix(GetControlRotation()).GetScaledAxis(EAxis::X);
	Forward.Z = 0.f;
	Forward.Normalize();

	AddMovementInput(Forward * Val);
	//MoveX = Val;
}

void ATotemPawn::AddYaw(float Val)
{
	if (!IsPaused) {
		AddControllerYawInput(Val);
	}
}

void ATotemPawn::AddPitch(float Val)
{
	if (!IsPaused) {
		AddControllerPitchInput(Val);
	}
}

void ATotemPawn::FirePressed()
{
	bWantsToFire = true;
}

void ATotemPawn::FireReleased()
{
	bWantsToFire = false;
}

void ATotemPawn::FireAltPressed()
{
	bWantsToFireAlt = true;
}

void ATotemPawn::FireAltReleased()
{
	bWantsToFireAlt = false;
}

void ATotemPawn::Weapon1() {
	TryEquipWeaponByIndex(0);
}

void ATotemPawn::Weapon2() {
	TryEquipWeaponByIndex(1);
}

void ATotemPawn::Weapon3() {
	TryEquipWeaponByIndex(2);
}

void ATotemPawn::OnHealthUpdate()
{
	// Client-specific functionality
	/*if (IsLocallyControlled()) {
		FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

		if (CurrentHealth <= 0)
		{
			FString deathMessage = FString::Printf(TEXT("You have been killed."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
		}
	}

	// Server-specific functionality
	if (GetLocalRole() == ROLE_Authority)
	{
		FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}*/

	UpdateHealth(CurrentHealth);
}
