// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverComponent.h"
#include <Runtime\Engine\Classes\Kismet\GameplayStatics.h>

UHoverComponent::UHoverComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UHoverComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = GetOwner();
	InitialLocation = GetOwner()->GetActorLocation();
}

// Called every frame
void UHoverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (Owner) {
		float ElapsedTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
		FVector Offset = FMath::Sin(ElapsedTime * HoverSpeed) * FVector::UpVector * HoverMagnitude;
		Owner->SetActorLocation(InitialLocation + Offset);
	}
}

