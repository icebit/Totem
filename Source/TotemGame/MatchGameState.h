#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MatchGameState.generated.h"

class UCountdownComponent;

UCLASS()
class TOTEMGAME_API AMatchGameState : public AGameState
{
	GENERATED_BODY()

public:
	AMatchGameState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Countdown")
	UCountdownComponent* CountdownComponent;

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = "Countdown")
	int32 CountdownValue;
};
