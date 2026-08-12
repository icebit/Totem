// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "PIDComponent.h"
#include "TakeHitInfo.h"
#include "GameFramework/Pawn.h"
#include "Components/TimelineComponent.h"
#include "GenericPlatform/GenericPlatform.h"
#include "TotemPawn.generated.h"

class UCurveFloat;

UCLASS(Config=Game)
class ATotemPawn : public APawn
{
	GENERATED_BODY()
public:
	ATotemPawn();

	// Totem mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* TotemMesh;

	// Kill Z
	UPROPERTY(Category = Death, EditAnywhere)
	float KillZ;

	UPROPERTY(EditAnywhere, Category = Movement)
	float ExplosionEffectTime;

	UPROPERTY(EditAnywhere, Category = Movement)
	float ExplosionEffectMaxDistance;
	
	// Camera
	UPROPERTY(Category = Camera, EditAnywhere)
	float CameraHeight;
	
	UPROPERTY(Category = Camera, EditAnywhere)
	float LookTorque;

	UPROPERTY(Category = Camera, EditAnywhere)
	float TargetCameraDistance;

	UPROPERTY(Category = Camera, EditAnywhere)
	float MinCameraDistance;

	UPROPERTY(Category = Camera, EditAnywhere)
	float MaxCameraDistance;

	// Used to set target camera distance from blueprint
	UFUNCTION(Category = Camera, BlueprintCallable)
	void SetTargetCameraDistance(float Distance);

	UPROPERTY(Category = Camera, EditAnywhere)
	float CameraDistanceLerp;

	UPROPERTY(Category = Camera, EditAnywhere)
	float LookSpeed;

	UPROPERTY(Category = Camera, EditAnywhere)
	float LookYMultiplier;

	UPROPERTY(Category = Rotation, EditAnywhere)
	float RotationTorque;

	// Spring arm that will offset the camera
	UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly)
	class USpringArmComponent* SpringArm;

	// Camera component that will be our viewpoint
	UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly)
	class UCameraComponent* Camera;
 
	// Weapons

	// Weapon mounting point
	UPROPERTY(Category = Weapons, VisibleAnywhere, BlueprintReadWrite)
	class USceneComponent* WeaponMountPoint;
		
	// Default inventory list
	UPROPERTY(Category = Weapons, EditDefaultsOnly)
	TArray<TSubclassOf<class ABaseWeapon>> DefaultInventoryClasses;

	// Current inventory
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Inventory)
	TArray<class ABaseWeapon*> Inventory;

	UFUNCTION(Category = Weapons, BlueprintCallable)
	TArray<class ABaseWeapon*> GetInventory();

	// Get currently equipped weapon
	UFUNCTION(BlueprintCallable, Category = Weapon)
	class ABaseWeapon* GetWeapon() const;
	
	// Server spawns default inventory
	UFUNCTION(BlueprintCallable, Category = Weapons)
	void SpawnDefaultInventory();

	UFUNCTION(BlueprintCallable, Category = Weapons)
	class ABaseWeapon* GiveWeapon(TSubclassOf<class ABaseWeapon> WeaponClass);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	class AShooterWeapon* GiveShooterWeapon(TSubclassOf<class AShooterWeapon> WeaponClass, int Ammo);

	// Server: Add wepaon to inventory
	UFUNCTION(BlueprintCallable, Category = Weapons)
	void AddWeapon(class ABaseWeapon* Weapon);

	// Server: Remove weapon from inventory
	UFUNCTION(BlueprintCallable, Category = Weapons)
	void RemoveWeapon(class ABaseWeapon* Weapon);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	class ABaseWeapon* FindWeapon(TSubclassOf<class ABaseWeapon> WeaponClass);

	// Server and client: Equip weapon
	UFUNCTION(BlueprintCallable, Category = Weapons)
	void EquipWeapon(class ABaseWeapon* Weapon);

	// Current weapon rep handler
	UFUNCTION()
	void OnRep_CurrentWeapon(class ABaseWeapon* LastWeapon);

	UFUNCTION()
	void OnRep_Inventory();
	UFUNCTION(BlueprintCallable)
	void OnNextWeapon();

	UFUNCTION(BlueprintCallable)
	void OnPrevWeapon();

	UFUNCTION(BlueprintCallable)
	int32 GetCurrentWeaponIndex();

	UPROPERTY(Category = Health, EditAnywhere)
	float RegenerationDelay;

	UPROPERTY(Category = Health, EditAnywhere)
	float RegenerationRate;

	// Getter for Max Health
	UFUNCTION(BlueprintPure, Category = Health)
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	// Getter for Current Health
	UFUNCTION(BlueprintPure, Category = Health)
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

	// Setter for Current Health. Clamps the value between 0 and MaxHealth and calls OnHealthUpdate. Should only be called on the server.
	UFUNCTION(BlueprintCallable, Category = Health)
	void SetCurrentHealth(float HealthValue);
	
	// Blueprint update health event
	UFUNCTION(BlueprintNativeEvent, Category = Health)
	void UpdateHealth(float NewHealth);

	// Event for taking damage. Overridden from APawn
	UFUNCTION(BlueprintCallable, Category = Health)
	float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(Category = Health)
	void BeginRegeneration();

	// When this is true, the totem's movement will be controlled
	// This is disabled when the totem is dead or in an urn
	UPROPERTY(BlueprintReadWrite)
	bool IsEnabled;

	// Functions to enable and disable character
	UFUNCTION(BlueprintCallable)
	void Enable();

	UFUNCTION(BlueprintCallable)
	void Disable();

	UFUNCTION(BlueprintCallable)
	void SetPaused(bool Paused);

	// Returns SpringArm subobject
	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArm; }

	// Returns Camera subobject
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }
	
	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Clean up inventory
	virtual void Destroyed() override;

	virtual void BeginPlay() override;

	virtual void PostInitProperties() override;

	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	virtual void Tick(float DeltaSeconds) override;
	//virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	// Is the player dashing at all?
	bool IsDashing;
protected:
	void DestroyInventory();

	UFUNCTION(BlueprintCallable, Category = Weapons)
	float GetScopeZoom();

	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon)
	class ABaseWeapon* CurrentWeapon;

	// Set current weapon
	void SetCurrentWeapon(class ABaseWeapon* NewWeapon, class ABaseWeapon* LastWeapon = NULL);

	// Replicate where this pawn was last hit and damaged
	UPROPERTY(Transient, ReplicatedUsing = OnRep_LastTakeHitInfo)
	struct FTakeHitInfo LastTakeHitInfo;

	UFUNCTION()
	void OnRep_LastTakeHitInfo();

	// Time at which point the last take hit info for the actor times out and won't be replicated; Used to stop join-in-progress effects all over the screen
	float LastTakeHitTimeTimeout;

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerEquipWeapon(class ABaseWeapon* NewWeapon);
	bool ServerEquipWeapon_Validate(class ABaseWeapon* NewWeapon);
	void ServerEquipWeapon_Implementation(class ABaseWeapon* NewWeapon);

	/*
	UFUNCTION(Unreliable, Server, WithValidation)
	void ServerDash(float SideDash, float ForwardDash);
	bool ServerDash_Validate(float SideDash, float ForwardDash);
	void ServerDash_Implementation(float SideDash, float ForwardDash);

	UFUNCTION(Unreliable, NetMulticast)
	void ClientDash(float SideDash, float ForwardDash);
	bool ClientDash_Validate(float SideDash, float ForwardDash);
	void ClientDash_Implementation(float SideDash, float ForwardDash);
	*/
	
	// Health
	UPROPERTY(Category = Health, EditDefaultsOnly)
	float MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	UFUNCTION()
	void OnRep_CurrentHealth();

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override; // Allows binding actions/axes to functions

	bool IsDead;
	
	float CurrentEnergy;

	UFUNCTION()
	bool Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser);

	UFUNCTION(BlueprintNativeEvent, Category = Death)
	void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser);

	UFUNCTION(BlueprintNativeEvent, Category = Weapons)
	void OnUpdateInventory();

	UFUNCTION(BlueprintNativeEvent, Category = Hover)
	void GroundEffects(FVector GroundPoint);

	void ReplicateHit(float Damage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, bool bKilled);

	// Input functions
	UFUNCTION(BlueprintCallable)
	void HorizontalInput(float Val);

	UFUNCTION(BlueprintCallable)
	void VerticalInput(float Val);

	void AddYaw(float Val);

	void AddPitch(float Val);
	
	void FirePressed();

	void FireReleased();

	void FireAltPressed();

	void FireAltReleased();

	void Weapon1();

	void Weapon2();
	
	void Weapon3();

	void TryEquipWeaponByIndex(int32 Index);

	// Called to fire the current weapon
	void FireCurrentWeapon();

	// Response to health being updated. Called on the server immediately after modification, and on clients in response to a RepNotify
	void OnHealthUpdate();
	
	void EndExplosionEffect();

	float DistanceFromExplosion;

	float HorizontalSpeed();
	
	// Explosion effect
	FTimerHandle ExplosionTimer;

	// Dash timer for recharging and finishing dash
	FTimerHandle DashTimer;

	// Dash timer for reequipping weapons
	FTimerHandle DashReequipTimer;
	
	// Regeneration timer
	FTimerHandle RegenerationTimer;

	FRotator InternalBaseRotation;

	float NeutralHoverPhase;
	
	// Current roll rotation from dash
	int CurrentSideDashDirection;
	int CurrentForwardDashDirection;

	float CurrentDashRoll;
	float CurrentDashPitch;

	// Move input axes
	float MoveX;
	float MoveY;

	// Is the aim button pressed?
	bool ShouldAim;

	// Is the dash button pressed?
	bool ShouldDash;

	bool IsPaused;

	bool IsGrounded;

	bool IsAiming;

	bool IsRegenerating;

	bool bIsFiring;
	
	bool bIsFiringAlt;

	bool bWantsToFire;
	
	bool bWantsToFireAlt;
};