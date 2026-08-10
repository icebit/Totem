#pragma once

#include "CoreMinimal.h"
#include "MatchPlayerController.h"
#include "TeamPlayerController.generated.h"

UCLASS()
class TOTEMGAME_API ATeamPlayerController : public AMatchPlayerController
{
	GENERATED_BODY()

public:
	ATeamPlayerController();

	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetTeam(int32 TeamIndex);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Team")
	void ServerChooseTeam(int32 TeamIndex);

	UFUNCTION(Client, Reliable)
	void ClientOnTeamSet(int32 TeamIndex);

	virtual void OnPossess(APawn* InPawn) override;

protected:
	UPROPERTY(Transient)
	int32 MyTeam;
};
