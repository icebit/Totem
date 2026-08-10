#pragma once

#include "CoreMinimal.h"
#include "TeamPlayerState.h"
#include "UrnWarsPlayerState.generated.h"

UCLASS()
class TOTEMGAME_API AUrnWarsPlayerState : public ATeamPlayerState
{
	GENERATED_BODY()

public:
	AUrnWarsPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = "UrnWars")
	int32 Souls;
};
