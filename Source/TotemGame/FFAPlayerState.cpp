#include "FFAPlayerState.h"

AFFAPlayerState::AFFAPlayerState()
{
	bIsReady = false;
}

void AFFAPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void AFFAPlayerState::OnRespawnCountdownTick_Implementation() {}
