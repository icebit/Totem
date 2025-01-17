// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

UCLASS(Abstract)
class TOTEM_API ABaseWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseWeapon();

	virtual void OnEquip(const ABaseWeapon* LastWeapon);

	// Weapon is now equipped by owner pawn
	virtual void OnEquipFinished();

	void OnUnequip();

	FTimerHandle OnEquipFinishedTimerHandle;

	float EquipStartedTime;

	UPROPERTY(EditAnywhere)
	float EquipDuration;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UTexture2D* Icon;

	// Inifite ammo
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Ammo)
	bool bInfiniteAmmo;

	// Max ammo
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	int32 MaxAmmo;

	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	int32 DefaultAmmo;

	UPROPERTY(Transient, Replicated)
	int32 CurrentAmmo;

	// Get current ammo amount
	UFUNCTION(BlueprintCallable)
	int32 GetCurrentAmmo() const;

	// Get max ammo
	UFUNCTION(BlueprintCallable)
	int32 GetMaxAmmo() const;

	UFUNCTION(BlueprintCallable)
	int32 GiveAmmo(int Amount);

	// Reload time
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	float ReloadTime;

	bool bIsEquipped;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_MyPawn)
	class ATotemPawn* MyPawn;

	UPROPERTY(EditDefaultsOnly, Category = Scope)
	float ScopeZoom;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	class UPrimitiveComponent* PawnPrimitiveComponent;

	virtual void DetermineWeaponState();
	
	bool bPendingEquip;

	UFUNCTION()
	void OnRep_MyPawn();

public:
	// Consume ammo
	void UseAmmo();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// [local + server] start weapon fire
	virtual void StartFire() PURE_VIRTUAL(ABaseWeapon::StartFire, );

	// [local + server] stop weapon fire
	virtual void StopFire() PURE_VIRTUAL(ABaseWeapon::StopFire, );

	// [local + server] start weapon fire
	virtual void StartFireAlt() PURE_VIRTUAL(ABaseWeapon::StartFireAlt, );

	// [local + server] stop weapon fire
	virtual void StopFireAlt() PURE_VIRTUAL(ABaseWeapon::StopFireAlt, );

	UFUNCTION(BlueprintCallable, Category = Inventory)
	void OnEnterInventory(ATotemPawn* NewOwner);

	void SetMyPawn(ATotemPawn* NewOwner);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	void OnLeaveInventory();

	UFUNCTION(BlueprintNativeEvent)
	void EquipEffects();

	UFUNCTION(Category = Ammo, BlueprintNativeEvent)
	void OutOfAmmoEffects();
};
