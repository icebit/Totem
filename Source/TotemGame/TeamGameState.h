#pragma once

#include "CoreMinimal.h"
#include "MatchGameState.h"
#include "TeamState.h"
#include "TeamGameState.generated.h"

USTRUCT(BlueprintType)
struct FTeamInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Team")
	FString TeamName;

	UPROPERTY(BlueprintReadWrite, Category = "Team")
	FLinearColor TeamColor;

	UPROPERTY(BlueprintReadWrite, Category = "Team")
	FLinearColor TeamAccentColor;
};

UCLASS()
class TOTEMGAME_API ATeamGameState : public AMatchGameState
{
	GENERATED_BODY()

public:
	ATeamGameState();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
	int32 NumTeams;

	UPROPERTY(BlueprintReadWrite, Category = "Team")
	TArray<FTeamInfo> TeamInfos;

	UPROPERTY(BlueprintReadWrite, Category = "Team")
	TArray<FTeamState> TeamStates;

	UFUNCTION(BlueprintCallable, Category = "Team")
	void InitializeTeams(int32 InNumTeams);

	UFUNCTION(BlueprintCallable, Category = "Team")
	int32 GetTeamScore(int32 TeamIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Team")
	bool IsTeamEliminated(int32 TeamIndex) const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	bool bHasInitialized;
};
