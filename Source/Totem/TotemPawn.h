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
class UAudioComponent;
class UTextRenderComponent;
class UNiagaraSystem;
class USoundCue;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class ABaseWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDissolveFinished);

UENUM(BlueprintType)
enum class EDashRotationAxis : uint8
{
	None,
	Roll,
	Pitch,
	Both
};

UCLASS(Config=Game)
class ATotemPawn : public APawn
{
	GENERATED_BODY()
public:
	ATotemPawn();

	// Totem mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* TotemMesh;

	// Scene component where movement forces are applied (prevents spin)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class USceneComponent* ForceApplication;

	// Scene component where dash impulse is applied
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class USceneComponent* DashForcePoint;

	// PID controller for hover height
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UPIDComponent* HoverPID;

	// Nametag text render
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UTextRenderComponent* Nametag;

	// Ambient loop audio
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UAudioComponent* AmbientLoop;

	// Outline mesh for team highlight
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* TotemOutline;

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

	UFUNCTION(Category = Camera, BlueprintCallable)
	void SetTargetCameraDistance(float Distance);

	UPROPERTY(Category = Camera, EditAnywhere)
	float CameraDistanceLerp;

	UPROPERTY(Category = Camera, EditAnywhere)
	float CameraFOVLerp;

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

	// Aim state
	UPROPERTY(Category = Camera, EditAnywhere)
	float NormalCameraDistance;

	UPROPERTY(Category = Camera, EditAnywhere)
	float AimingCameraDistance;

	UPROPERTY(Category = Camera, EditAnywhere)
	float NormalCameraFOV;

	UPROPERTY(Category = Camera, EditAnywhere)
	float AimingCameraFOV;

	UPROPERTY(Category = Camera, EditAnywhere)
	float NormalSpeed;

	UPROPERTY(Category = Camera, EditAnywhere)
	float AimingSpeed;

	UPROPERTY(Category = Camera, EditAnywhere)
	float NormalHorizontalDamping;

	UPROPERTY(Category = Camera, EditAnywhere)
	float AimingHorizontalDamping;

	UPROPERTY(Category = Camera, EditAnywhere)
	float NormalVerticalDamping;

	UPROPERTY(Category = Camera, EditAnywhere)
	float AimingVerticalDamping;

	UPROPERTY(Category = Camera, EditAnywhere)
	float InFieldOfView;

	// Hover
	UPROPERTY(Category = Hover, EditAnywhere)
	float DesiredHoverHeight;

	UPROPERTY(Category = Hover, EditAnywhere)
	float MaxHoverHeight;

	UPROPERTY(Category = Hover, EditAnywhere)
	float HoverForce;

	UPROPERTY(Category = Hover, EditAnywhere)
	float ProportionalCoefficient;

	UPROPERTY(Category = Hover, EditAnywhere)
	float DerivativeCoefficient;

	UPROPERTY(Category = Hover, EditAnywhere)
	float PitchStabilization;

	UPROPERTY(Category = Hover, EditAnywhere)
	float RollStabilization;

	UPROPERTY(Category = Hover, EditAnywhere)
	float YawStabilization;

	UPROPERTY(Category = Hover, EditAnywhere)
	float DesiredRollAngle;

	UPROPERTY(Category = Hover, EditAnywhere)
	float AlignSpeed;

	UPROPERTY(Category = Hover, BlueprintReadOnly, Transient)
	bool IsGrounded;

	UPROPERTY(Category = Hover, EditAnywhere)
	UNiagaraSystem* GroundedFX;

	// Movement
	UPROPERTY(Category = Movement, EditAnywhere)
	float MovementForce;

	UPROPERTY(Category = Movement, EditAnywhere)
	float AngularDamping;

	UPROPERTY(Category = Movement, EditAnywhere)
	UCurveFloat* AccelerationCurve;

	UPROPERTY(Category = Movement, EditAnywhere)
	float SpeedSquaredAtMaxWindVolume;

	// Dash
	UPROPERTY(Category = Dash, EditAnywhere)
	float DashForce;

	UPROPERTY(Category = Dash, EditAnywhere)
	float DashDuration;

	UPROPERTY(Category = Dash, EditAnywhere)
	float DashForwardAxisMultiplier;

	UPROPERTY(Category = Dash, EditAnywhere)
	float DashRightAxisMultiplier;

	UPROPERTY(Category = Dash, EditAnywhere)
	float DashUpAxisMultiplier;

	UPROPERTY(Category = Dash, EditAnywhere)
	UCurveFloat* DashCurve;

	UPROPERTY(Category = Dash, EditAnywhere)
	UCurveFloat* DashCompletionCurve;

	UPROPERTY(Category = Dash, EditAnywhere)
	EDashRotationAxis DashRotationAxis;

	UPROPERTY(Category = Dash, EditAnywhere, BlueprintReadOnly)
	UNiagaraSystem* DashEffects;

	UPROPERTY(Category = Dash, EditAnywhere, BlueprintReadOnly)
	UNiagaraSystem* DashFX;

	UPROPERTY(Category = Dash, EditAnywhere, BlueprintReadOnly)
	UNiagaraSystem* DashTrails;

	UPROPERTY(ReplicatedUsing = OnRep_DashEffects, Transient, BlueprintReadOnly, Category = Dash)
	bool bDashEffectsPending;

	UFUNCTION()
	void OnRep_DashEffects();

	UFUNCTION(BlueprintNativeEvent, Category = Dash)
	void SpawnDashEffects();

	UFUNCTION(BlueprintCallable, Category = Dash)
	void StopDash();

	UFUNCTION(BlueprintNativeEvent, Category = Dash)
	void RechargeDash();

	UPROPERTY(Category = Dash, BlueprintReadOnly, Transient)
	bool IsDashing;

	UPROPERTY(Category = Dash, BlueprintReadOnly, Transient)
	bool WantsToDash;

	// Weapons
	UPROPERTY(Category = Weapons, VisibleAnywhere, BlueprintReadWrite)
	class USceneComponent* WeaponMountPoint;

	UPROPERTY(Category = Weapons, EditDefaultsOnly)
	TArray<TSubclassOf<class ABaseWeapon>> DefaultInventoryClasses;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_Inventory)
	TArray<class ABaseWeapon*> Inventory;

	UFUNCTION(Category = Weapons, BlueprintCallable)
	TArray<class ABaseWeapon*> GetInventory();

	UFUNCTION(BlueprintCallable, Category = Weapon)
	class ABaseWeapon* GetWeapon() const;

	UFUNCTION(BlueprintCallable, Category = Weapons)
	void SpawnDefaultInventory();

	UFUNCTION(BlueprintCallable, Category = Weapons)
	class ABaseWeapon* GiveWeapon(TSubclassOf<class ABaseWeapon> WeaponClass);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	class AShooterWeapon* GiveShooterWeapon(TSubclassOf<class AShooterWeapon> WeaponClass, int Ammo);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	void AddWeapon(class ABaseWeapon* Weapon);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	void RemoveWeapon(class ABaseWeapon* Weapon);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	class ABaseWeapon* FindWeapon(TSubclassOf<class ABaseWeapon> WeaponClass);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	void EquipWeapon(class ABaseWeapon* Weapon);

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

	UFUNCTION(BlueprintPure, Category = Health)
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = Health)
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category = Health)
	void SetCurrentHealth(float HealthValue);

	UFUNCTION(BlueprintNativeEvent, Category = Health)
	void UpdateHealth(float NewHealth);

	UFUNCTION(BlueprintCallable, Category = Health)
	float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(Category = Health)
	void BeginRegeneration();

	UPROPERTY(BlueprintReadWrite)
	bool IsEnabled;

	UFUNCTION(BlueprintCallable)
	void Enable();

	UFUNCTION(BlueprintCallable)
	void Disable();

	UFUNCTION(BlueprintCallable)
	void SetPaused(bool Paused);

	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArm; }
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void Destroyed() override;
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	// Dissolve
	UPROPERTY(BlueprintAssignable, Category = Death)
	FOnDissolveFinished OnDissolveFinished;

	UPROPERTY(Category = Death, EditAnywhere)
	UMaterialInterface* DissolveMaterial;

	UPROPERTY(Category = Death, EditAnywhere)
	UCurveFloat* DissolveCurve;

	UPROPERTY(Transient, BlueprintReadOnly, Category = Death)
	UMaterialInstanceDynamic* DissolveMID;

	UFUNCTION(BlueprintCallable, Category = Death)
	void DissolveIn();

	UFUNCTION(BlueprintCallable, Category = Death)
	void DissolveOut();

	UFUNCTION()
	void DissolveTimelineUpdate(float Value);

	UFUNCTION()
	void DissolveTimelineFinished();

	// Death sounds
	UPROPERTY(Category = Death, EditAnywhere)
	USoundCue* YouDiedSoundCue;

	UPROPERTY(Category = Death, EditAnywhere)
	USoundCue* OtherDiedSoundCue;

	// Interact
	UPROPERTY(Category = Interact, EditAnywhere)
	float MaxInteractDistance;

	UPROPERTY(Category = Interact, BlueprintReadOnly, Transient)
	bool IsInteracting;

	UFUNCTION(BlueprintCallable, Category = Interact)
	void CheckInteract();

	UFUNCTION(BlueprintNativeEvent, Category = Interact)
	void OnInteract(AActor* Interactable);

	// Nametag
	UPROPERTY(Category = Nametag, EditAnywhere)
	float NametagHeight;

	UPROPERTY(Category = Nametag, EditAnywhere)
	UCurveFloat* FadeNametagCurve;

	UFUNCTION(BlueprintCallable, Category = Nametag)
	void SetName(const FString& NewName);

	UFUNCTION(BlueprintCallable, Category = Nametag)
	void SetAccentColor(FLinearColor Color);

	UFUNCTION(BlueprintCallable, Category = Nametag)
	void SetOutlineVisibility(bool bVisible);

	UFUNCTION(BlueprintNativeEvent, Category = Nametag)
	void ShouldUpdateNametag();

	UFUNCTION()
	void FadeNametagUpdate(float Value);

	UFUNCTION()
	void FadeNametagFinished();

	UPROPERTY(Transient, BlueprintReadOnly, Category = Nametag)
	bool bNametagVisible;

	UPROPERTY(Category = Nametag, Transient, BlueprintReadOnly)
	FTimeline FadeNametagTimeline;

	// Team
	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = Team)
	int32 Team;

	UFUNCTION(BlueprintCallable, Category = Team)
	void SetTeam(int32 NewTeam);

protected:
	void DestroyInventory();

	UFUNCTION(BlueprintCallable, Category = Weapons)
	float GetScopeZoom();

	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon)
	class ABaseWeapon* CurrentWeapon;

	void SetCurrentWeapon(class ABaseWeapon* NewWeapon, class ABaseWeapon* LastWeapon = NULL);

	UPROPERTY(Transient, ReplicatedUsing = OnRep_LastTakeHitInfo)
	struct FTakeHitInfo LastTakeHitInfo;

	UFUNCTION()
	void OnRep_LastTakeHitInfo();

	float LastTakeHitTimeTimeout;

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerEquipWeapon(class ABaseWeapon* NewWeapon);
	bool ServerEquipWeapon_Validate(class ABaseWeapon* NewWeapon);
	void ServerEquipWeapon_Implementation(class ABaseWeapon* NewWeapon);

	UPROPERTY(Category = Health, EditDefaultsOnly)
	float MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	UFUNCTION()
	void OnRep_CurrentHealth();

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

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

	UFUNCTION(BlueprintNativeEvent, Category = Pawn)
	void ReceivePossessed(AController* NewController);

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

	void DashPressed();
	void DashReleased();

	void AimPressed();
	void AimReleased();

	void InteractPressed();

	void Weapon1();
	void Weapon2();
	void Weapon3();

	void TryEquipWeaponByIndex(int32 Index);
	void FireCurrentWeapon();

	void OnHealthUpdate();
	void EndExplosionEffect();

	float DistanceFromExplosion;
	float HorizontalSpeed();

	// Explosion effect
	FTimerHandle ExplosionTimer;

	// Dash timers
	FTimerHandle DashTimer;
	FTimerHandle DashReequipTimer;

	// Regeneration timer
	FTimerHandle RegenerationTimer;

	// Dissolve timeline
	UPROPERTY(Transient)
	FTimeline DissolveTimeline;

	FRotator InternalBaseRotation;
	float NeutralHoverPhase;

	int CurrentSideDashDirection;
	int CurrentForwardDashDirection;
	float CurrentDashRoll;
	float CurrentDashPitch;

	// Move input axes
	float MoveX;
	float MoveY;

	bool ShouldAim;
	bool ShouldDash;
	bool IsPaused;
	bool IsAiming;
	bool IsRegenerating;
	bool bIsFiring;
	bool bIsFiringAlt;
	bool bWantsToFire;
	bool bWantsToFireAlt;

	// Whether the pawn has been frozen after death
	bool bFrozen;

	void BeginDash();
	void ApplyHoverForce(float DeltaSeconds);
	void ApplyMovementForce(float DeltaSeconds);
	void ApplyStabilizationTorque(float DeltaSeconds);
	void UpdateCameraState(float DeltaSeconds);
	void UpdateWindVolume(float DeltaSeconds);
	void UpdateNametag(float DeltaSeconds);
	void Freeze();

	// Trace channel for hover
	UPROPERTY(EditAnywhere, Category = Hover)
	TEnumAsByte<ECollisionChannel> HoverTraceChannel;
};
