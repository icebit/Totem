#pragma once

#include "CoreMinimal.h"
#include "MatchPlayerState.h"
#include "TeamPlayerState.generated.h"

UCLASS()
class TOTEMGAME_API ATeamPlayerState : public AMatchPlayerState
{
	GENERATED_BODY()

public:
	ATeamPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = "Team")
	int32 Team;

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = "Team")
	bool bIsEliminated;

	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetTeam(int32 NewTeam);
};
