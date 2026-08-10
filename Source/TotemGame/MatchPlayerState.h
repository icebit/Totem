#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MatchPlayerState.generated.h"

class UCountdownComponent;

UCLASS()
class TOTEMGAME_API AMatchPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AMatchPlayerState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Countdown")
	UCountdownComponent* CountdownComponent;

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = "Match")
	bool bIsReady;

	UFUNCTION(BlueprintCallable, Category = "Match")
	void SetReady(bool bReady);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Match")
	void ServerReady();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Match")
	void ServerSetName(const FString& NewName);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UFUNCTION()
	void OnRep_Name();

	UPROPERTY(ReplicatedUsing = OnRep_Name, Transient)
	FString PlayerName;
};
