#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Urn.generated.h"

UCLASS()
class TOTEMGAME_API AUrn : public AActor
{
	GENERATED_BODY()

public:
	AUrn();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = "Urn")
	bool bIsDestroyed;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadOnly, Category = "Urn")
	int32 TeamIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Urn")
	float MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_Health, Transient, BlueprintReadOnly, Category = "Urn")
	float CurrentHealth;

	UFUNCTION(BlueprintNativeEvent, Category = "Urn")
	void OnUrnDestroyed(AController* Destroyer);

	UFUNCTION(BlueprintNativeEvent, Category = "Urn")
	void OnHealthChanged(float NewHealth);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UFUNCTION()
	void OnRep_Health();
};
