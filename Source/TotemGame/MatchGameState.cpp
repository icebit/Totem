#include "MatchGameState.h"
#include "CountdownComponent.h"

AMatchGameState::AMatchGameState()
{
	CountdownComponent = CreateDefaultSubobject<UCountdownComponent>(TEXT("CountdownComponent"));
}
