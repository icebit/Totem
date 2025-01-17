// Fill out your copyright notice in the Description page of Project Settings.


#include "AINavMovementComponent.h"

void UAINavMovementComponent::RequestPathMove(const FVector& MoveInput)
{
}

void UAINavMovementComponent::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
    CurrentMoveVelocity = MoveVelocity;
}

FVector UAINavMovementComponent::GetMoveVelocity()
{
    return CurrentMoveVelocity;
}
