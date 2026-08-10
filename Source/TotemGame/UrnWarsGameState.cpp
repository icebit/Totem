#include "UrnWarsGameState.h"
#include "Urn.h"
#include "Kismet/GameplayStatics.h"

AUrnWarsGameState::AUrnWarsGameState()
{
}

void AUrnWarsGameState::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> UrnActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUrn::StaticClass(), UrnActors);

	Urns.SetNum(NumTeams);
	for (AActor* UrnActor : UrnActors)
	{
		AUrn* Urn = Cast<AUrn>(UrnActor);
		if (Urn)
		{
			int32 TeamIndex = Urn->TeamIndex;
			if (Urns.IsValidIndex(TeamIndex))
			{
				Urns[TeamIndex] = Urn;
			}
		}
	}
}

AUrn* AUrnWarsGameState::GetUrn(int32 TeamIndex) const
{
	if (!Urns.IsValidIndex(TeamIndex))
	{
		return nullptr;
	}
	return Urns[TeamIndex];
}

bool AUrnWarsGameState::IsUrnDestroyed(int32 TeamIndex) const
{
	AUrn* Urn = GetUrn(TeamIndex);
	return Urn ? Urn->bIsDestroyed : false;
}
