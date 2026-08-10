#include "TeamPlayerState.h"
#include "Net/UnrealNetwork.h"

ATeamPlayerState::ATeamPlayerState()
{
	Team = -1;
	bIsEliminated = false;
}

void ATeamPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATeamPlayerState, Team);
	DOREPLIFETIME(ATeamPlayerState, bIsEliminated);
}

void ATeamPlayerState::SetTeam(int32 NewTeam)
{
	Team = NewTeam;
}
