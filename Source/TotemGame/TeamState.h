#pragma once

#include "CoreMinimal.h"
#include "TeamState.generated.h"

USTRUCT(BlueprintType)
struct FTeamState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Team")
	int32 TotalSpawns = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Team")
	bool bIsEliminated = false;

	UPROPERTY(BlueprintReadWrite, Category = "Team")
	TArray<AActor*> TeamStarts;

	UPROPERTY(BlueprintReadWrite, Category = "Team")
	int32 UniqueNameId = 0;
};
