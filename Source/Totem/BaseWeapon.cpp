// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseWeapon.h"
#include "TotemPawn.h"
#include <Runtime\Engine\Public\Net\UnrealNetwork.h>

// Sets default values
ABaseWeapon::ABaseWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bOnlyRelevantToOwner = false;
	bNetUseOwnerRelevancy = true;

	bIsEquipped = false;
	EquipDuration = 0.3f;

	CurrentAmmo = 0;
	bInfiniteAmmo = false;

	ScopeZoom = 1.0f;

	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	// If is authority, give ammo
	if (GetLocalRole() == ROLE_Authority) {
		if (DefaultAmmo > 0) {
			GiveAmmo(DefaultAmmo);
		}
	}
	else {
		// Otherwise just set ammo, this will be corrected by the server if it is somehow wrong
		CurrentAmmo = DefaultAmmo;
	}
}

int32 ABaseWeapon::GetCurrentAmmo() const
{
	return CurrentAmmo;
}

int32 ABaseWeapon::GetMaxAmmo() const
{
	return MaxAmmo;
}

int32 ABaseWeapon::GiveAmmo(int Amount)
{
	const int32 MissingAmmo = FMath::Max(0, MaxAmmo - CurrentAmmo);
	Amount = FMath::Min(Amount, MissingAmmo);
	CurrentAmmo += Amount;

	/*AShooterAIController* BotAI = MyPawn ? Cast<AShooterAIController>(MyPawn->GetController()) : NULL;
	if (BotAI)
	{
		BotAI->CheckAmmo(this);
	}*/

	// start reload if clip was empty
	/*if (GetCurrentAmmoInClip() <= 0 &&
		CanReload() &&
		MyPawn->GetWeapon() == this)
	{
		ClientStartReload();
	}*/

	return Amount;
}

void ABaseWeapon::UseAmmo()
{
	if (!bInfiniteAmmo)
	{
		CurrentAmmo--;
	}
}

void ABaseWeapon::DetermineWeaponState()
{
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseWeapon::OnEnterInventory(ATotemPawn* NewOwner) {
	SetMyPawn(NewOwner);
}

void ABaseWeapon::OnLeaveInventory()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		SetMyPawn(NULL);
	}
}

void ABaseWeapon::EquipEffects_Implementation()
{

}

void ABaseWeapon::OnEquip(const ABaseWeapon* LastWeapon)
{
	bPendingEquip = true;

	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);

	if (MyPawn) {
		AttachToComponent(MyPawn->WeaponMountPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	DetermineWeaponState();

	// Only play animation if last weapon is valid
	if (LastWeapon)
	{
		if (EquipDuration == 0.f) {
			OnEquipFinished();
		}
		else {
			GetWorldTimerManager().SetTimer(OnEquipFinishedTimerHandle, this, &ABaseWeapon::OnEquipFinished, EquipDuration, false);
		}
	}
	else
	{
		OnEquipFinished();
	}

	EquipEffects();
}

void ABaseWeapon::OnEquipFinished()
{
	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);
	bIsEquipped = true;
	bPendingEquip = false;
	DetermineWeaponState();
}

void ABaseWeapon::OnUnequip()
{
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
	bIsEquipped = false;
	StopFire();
	StopFireAlt();

	if (bPendingEquip)
	{
		bPendingEquip = false;

		GetWorldTimerManager().ClearTimer(OnEquipFinishedTimerHandle);
	}

	DetermineWeaponState();
}

void ABaseWeapon::SetMyPawn(ATotemPawn* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		SetInstigator(NewOwner);
		MyPawn = NewOwner;
		// net owner for RPC calls
		SetOwner(NewOwner);
	}

	if (NewOwner) {
		PawnPrimitiveComponent = Cast<UPrimitiveComponent>(NewOwner->GetRootComponent());
	}
}

void ABaseWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ABaseWeapon, CurrentAmmo, COND_OwnerOnly);
	DOREPLIFETIME(ABaseWeapon, MyPawn);
}

void ABaseWeapon::OutOfAmmoEffects_Implementation()
{
}

void ABaseWeapon::OnRep_MyPawn()
{
	if (MyPawn)
	{
		OnEnterInventory(MyPawn);
	}
	else
	{
		OnLeaveInventory();
	}
}
