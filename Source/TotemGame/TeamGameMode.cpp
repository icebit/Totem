#include "TeamGameMode.h"
#include "TeamPlayerState.h"
#include "TeamGameState.h"
#include "TeamPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ATeamGameMode::ATeamGameMode()
{
	NumTeams = 2;
	ScoreLimit = 25;
}

void ATeamGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (GetLocalRole() == ROLE_Authority)
	{
		ATeamGameState* TeamGS = GetGameState<ATeamGameState>();
		if (TeamGS && !TeamGS->TeamInfos.Num())
		{
			TeamGS->InitializeTeams(NumTeams);
		}

		int32 AssignedTeam = GetSmallestTeam();
		ChooseTeam(NewPlayer, AssignedTeam);
	}
}

bool ATeamGameMode::CanDamage_Implementation(AController* Attacker, AController* Victim)
{
	if (!Attacker || !Victim)
	{
		return true;
	}

	ATeamPlayerState* AttackerPS = Cast<ATeamPlayerState>(Attacker->PlayerState);
	ATeamPlayerState* VictimPS = Cast<ATeamPlayerState>(Victim->PlayerState);

	if (AttackerPS && VictimPS)
	{
		return AttackerPS->Team != VictimPS->Team;
	}

	return true;
}

void ATeamGameMode::Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	Super::Killed(Killer, KilledPlayer, KilledPawn, DamageType);
	CheckGameOver();
}

AActor* ATeamGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	ATeamPlayerState* TeamPS = Cast<ATeamPlayerState>(Player ? Player->PlayerState : nullptr);
	if (!TeamPS)
	{
		return Super::FindPlayerStart_Implementation(Player, IncomingName);
	}

	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

	TArray<AActor*> TeamStarts;
	for (AActor* Start : PlayerStarts)
	{
		if (Start->ActorHasTag(FName(*FString::FromInt(TeamPS->Team))))
		{
			TeamStarts.Add(Start);
		}
	}

	if (TeamStarts.Num() > 0)
	{
		return TeamStarts[FMath::RandRange(0, TeamStarts.Num() - 1)];
	}

	return Super::FindPlayerStart_Implementation(Player, IncomingName);
}

void ATeamGameMode::ChooseTeam(AController* Player, int32 TeamIndex)
{
	ATeamPlayerController* TeamPC = Cast<ATeamPlayerController>(Player);
	if (TeamPC)
	{
		TeamPC->ServerChooseTeam(TeamIndex);
	}
}

int32 ATeamGameMode::GetSmallestTeam()
{
	TArray<int32> TeamCounts;
	TeamCounts.SetNum(NumTeams);

	for (APlayerState* PS : GameState->PlayerArray)
	{
		ATeamPlayerState* TeamPS = Cast<ATeamPlayerState>(PS);
		if (TeamPS && TeamPS->Team >= 0 && TeamPS->Team < NumTeams)
		{
			TeamCounts[TeamPS->Team]++;
		}
	}

	int32 Smallest = 0;
	for (int32 i = 1; i < NumTeams; i++)
	{
		if (TeamCounts[i] < TeamCounts[Smallest])
		{
			Smallest = i;
		}
	}
	return Smallest;
}

void ATeamGameMode::CheckGameOver()
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

void ATeamGameMode::InitializeTeams()
{
	ATeamGameState* TeamGS = GetGameState<ATeamGameState>();
	if (TeamGS)
	{
		TeamGS->InitializeTeams(NumTeams);
	}
}
