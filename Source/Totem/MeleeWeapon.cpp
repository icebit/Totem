// Fill out your copyright notice in the Description page of Project Settings.

#include "MeleeWeapon.h"
#include "TotemPawn.h"

AMeleeWeapon::AMeleeWeapon() {
	Base = CreateDefaultSubobject<USceneComponent>(TEXT("Base"));
	RootComponent = Base;

	LeftHand = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHand"));
	LeftHand->SetupAttachment(RootComponent);

	RightHand = CreateDefaultSubobject<USceneComponent>(TEXT("RightHand"));
	RightHand->SetupAttachment(RootComponent);

	CanSwing = true;

	SwingTime = 0.3f;

	SwingForce = 2000.f;

	ForceLocationLerp = 0.4f;
}

void AMeleeWeapon::Swing(bool left) {
	GEngine->AddOnScreenDebugMessage(-1, 0.4, FColor::Blue, "Swing");

	if (CanSwing && MyPawn->IsEnabled && MyPawn->IsDashing) {
		FVector HandLocation = left ? LeftHand->GetComponentLocation() : RightHand->GetComponentLocation();
		FVector ForceLocation = FMath::Lerp(GetActorLocation(), HandLocation, ForceLocationLerp);
		FVector ForceDirection = GetActorForwardVector();
		ForceDirection.Z = 0.f;
		ForceDirection.Normalize();

		if (PawnPrimitiveComponent) {
			GEngine->AddOnScreenDebugMessage(-1, 0.4, FColor::Blue, "Primitive");
			PawnPrimitiveComponent->AddImpulseAtLocation(ForceDirection * SwingForce, ForceLocation);
		}

		GetWorldTimerManager().SetTimer(SwingTimerHandle, this, &AMeleeWeapon::ResetSwing, SwingTime, false);

		CanSwing = false;
	}
}

void AMeleeWeapon::ResetSwing()
{
	CanSwing = true;
}

void AMeleeWeapon::StartFire() {
	Swing(true);
}

void AMeleeWeapon::StopFire() {

}

void AMeleeWeapon::StartFireAlt() {
	Swing(false);
}

void AMeleeWeapon::StopFireAlt() {

}