#include "TeamPlayerController.h"
#include "TeamPlayerState.h"
#include "TeamGameState.h"

ATeamPlayerController::ATeamPlayerController()
{
	MyTeam = -1;
}

void ATeamPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void ATeamPlayerController::SetTeam(int32 TeamIndex)
{
	MyTeam = TeamIndex;

	ATeamPlayerState* TeamPS = GetPlayerState<ATeamPlayerState>();
	if (TeamPS)
	{
		TeamPS->SetTeam(TeamIndex);
	}
}

bool ATeamPlayerController::ServerChooseTeam_Validate(int32 TeamIndex) { return TeamIndex >= 0; }
void ATeamPlayerController::ServerChooseTeam_Implementation(int32 TeamIndex)
{
	SetTeam(TeamIndex);
	ClientOnTeamSet(TeamIndex);
}

void ATeamPlayerController::ClientOnTeamSet_Implementation(int32 TeamIndex)
{
	MyTeam = TeamIndex;
}
