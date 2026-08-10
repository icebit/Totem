#pragma once

#include "CoreMinimal.h"
#include "TeamGameState.h"
#include "UrnWarsGameState.generated.h"

class AUrn;

UCLASS()
class TOTEMGAME_API AUrnWarsGameState : public ATeamGameState
{
	GENERATED_BODY()

public:
	AUrnWarsGameState();

	virtual void BeginPlay() override;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "UrnWars")
	TArray<AUrn*> Urns;

	UFUNCTION(BlueprintCallable, Category = "UrnWars")
	AUrn* GetUrn(int32 TeamIndex) const;

	UFUNCTION(BlueprintCallable, Category = "UrnWars")
	bool IsUrnDestroyed(int32 TeamIndex) const;
};
