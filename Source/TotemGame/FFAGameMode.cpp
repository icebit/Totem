#include "FFAGameMode.h"
#include "TotemPlayerState.h"
#include "MatchPlayerState.h"

AFFAGameMode::AFFAGameMode()
{
	ScoreLimit = 25;
}

bool AFFAGameMode::CanDamage_Implementation(AController* Attacker, AController* Victim)
{
	return Attacker != Victim;
}

void AFFAGameMode::Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	Super::Killed(Killer, KilledPlayer, KilledPawn, DamageType);
	CheckGameOver();
}

void AFFAGameMode::CheckGameOver()
{
	if (MatchState != EMatchState::InProgress)
	{
		return;
	}

	for (APlayerState* PS : GameState->PlayerArray)
	{
		ATotemPlayerState* TotemPS = Cast<ATotemPlayerState>(PS);
		if (TotemPS && TotemPS->GetKills() >= ScoreLimit)
		{
			EndMatch();
			return;
		}
	}
}
