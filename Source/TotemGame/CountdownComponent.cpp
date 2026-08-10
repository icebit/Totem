#include "CountdownComponent.h"
#include "Net/UnrealNetwork.h"

UCountdownComponent::UCountdownComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bIsCountingDown = false;
	InitialCountdown = 3;
	CountdownValue = 0;
	SetIsReplicatedByDefault(true);
}

void UCountdownComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCountdownComponent, CountdownValue);
}

void UCountdownComponent::BeginCountdown()
{
	CountdownValue = InitialCountdown;
	bIsCountingDown = true;
}

void UCountdownComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsCountingDown)
	{
		return;
	}

	AccumulatedTime += DeltaTime;
	if (AccumulatedTime >= 1.0f)
	{
		AccumulatedTime = 0.f;
		CountdownValue = FMath::Max(0, CountdownValue - 1);
		if (CountdownValue <= 0)
		{
			bIsCountingDown = false;
			OnEndCountdown.Broadcast();
		}
	}
}
