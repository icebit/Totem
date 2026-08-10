#pragma once

#include "CoreMinimal.h"
#include "MatchPlayerState.h"
#include "FFAPlayerState.generated.h"

UCLASS()
class TOTEMGAME_API AFFAPlayerState : public AMatchPlayerState
{
	GENERATED_BODY()

public:
	AFFAPlayerState();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent, Category = "FFA")
	void OnRespawnCountdownTick();

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = "FFA")
	bool bIsReady;
};
