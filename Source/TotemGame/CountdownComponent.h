#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CountdownComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCountdownEnded);

UCLASS(ClassGroup = "Totem", meta = (BlueprintSpawnableComponent))
class TOTEMGAME_API UCountdownComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCountdownComponent();

	UPROPERTY(BlueprintAssignable, Category = "Countdown")
	FOnCountdownEnded OnEndCountdown;

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = "Countdown")
	int32 CountdownValue;

	UPROPERTY(EditAnywhere, Category = "Countdown")
	int32 InitialCountdown;

	UFUNCTION(BlueprintCallable, Category = "Countdown")
	void BeginCountdown();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	bool bIsCountingDown;
	float AccumulatedTime;
};
