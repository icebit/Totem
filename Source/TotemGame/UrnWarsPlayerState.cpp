#include "UrnWarsPlayerState.h"
#include "Net/UnrealNetwork.h"

AUrnWarsPlayerState::AUrnWarsPlayerState()
{
	Souls = 0;
}

void AUrnWarsPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUrnWarsPlayerState, Souls);
}
