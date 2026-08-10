#include "TeamGameState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

ATeamGameState::ATeamGameState()
{
	NumTeams = 2;
	bHasInitialized = false;
	PrimaryActorTick.bCanEverTick = true;
}

void ATeamGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATeamGameState, NumTeams);
}

void ATeamGameState::BeginPlay()
{
	Super::BeginPlay();
}

void ATeamGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ATeamGameState::InitializeTeams(int32 InNumTeams)
{
	NumTeams = InNumTeams;
	TeamInfos.SetNum(NumTeams);
	TeamStates.SetNum(NumTeams);

	static const TArray<FString> DefaultNames = { TEXT("Red"), TEXT("Blue"), TEXT("Green"), TEXT("Yellow") };
	static const TArray<FLinearColor> DefaultColors = {
		FLinearColor::Red, FLinearColor::Blue, FLinearColor::Green, FLinearColor::Yellow
	};
	static const TArray<FLinearColor> DefaultAccents = {
		FLinearColor(1.f, 0.3f, 0.3f), FLinearColor(0.3f, 0.3f, 1.f),
		FLinearColor(0.3f, 1.f, 0.3f), FLinearColor(1.f, 1.f, 0.3f)
	};

	for (int32 i = 0; i < NumTeams; i++)
	{
		TeamInfos[i].TeamName = DefaultNames.IsValidIndex(i) ? DefaultNames[i] : FString::FromInt(i);
		TeamInfos[i].TeamColor = DefaultColors.IsValidIndex(i) ? DefaultColors[i] : FLinearColor::White;
		TeamInfos[i].TeamAccentColor = DefaultAccents.IsValidIndex(i) ? DefaultAccents[i] : FLinearColor::White;
	}

	bHasInitialized = true;
}

int32 ATeamGameState::GetTeamScore(int32 TeamIndex) const
{
	if (!TeamStates.IsValidIndex(TeamIndex))
	{
		return 0;
	}
	return TeamStates[TeamIndex].TotalSpawns;
}

bool ATeamGameState::IsTeamEliminated(int32 TeamIndex) const
{
	if (!TeamStates.IsValidIndex(TeamIndex))
	{
		return false;
	}
	return TeamStates[TeamIndex].bIsEliminated;
}
