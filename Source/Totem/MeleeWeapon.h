// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "MeleeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class TOTEM_API AMeleeWeapon : public ABaseWeapon
{
	GENERATED_BODY()

public:
	AMeleeWeapon();

	void StartFire() override;

	void StopFire() override;

	void StartFireAlt() override;

	void StopFireAlt() override;

	void Swing(bool left);

	void ResetSwing();

	FTimerHandle SwingTimerHandle;

	bool CanSwing;

	UPROPERTY(EditAnywhere)
	float SwingTime;

	UPROPERTY(EditAnywhere)
	float SwingForce;

	UPROPERTY(EditAnywhere)
	float ForceLocationLerp;

protected:
	UPROPERTY()
	class USceneComponent* Base;
	
	UPROPERTY(EditAnywhere)
	class USceneComponent* LeftHand;

	UPROPERTY(EditAnywhere)
	class USceneComponent* RightHand;
};
