// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/NavMovementComponent.h"
#include "AINavMovementComponent.generated.h"

/**
 * 
 */
UCLASS(meta=(BlueprintSpawnableComponent))
class TOTEM_API UAINavMovementComponent : public UNavMovementComponent
{
	GENERATED_BODY()
	

	virtual void RequestPathMove(const FVector& MoveInput) override;

	virtual void RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed) override;

	FVector CurrentMoveVelocity;
	
	UFUNCTION(BlueprintCallable)
	FVector GetMoveVelocity();
};
