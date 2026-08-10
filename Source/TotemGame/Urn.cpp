#include "Urn.h"
#include "Net/UnrealNetwork.h"

AUrn::AUrn()
{
	bReplicates = true;
	bIsDestroyed = false;
	TeamIndex = 0;
	MaxHealth = 1000.f;
	CurrentHealth = MaxHealth;
}

void AUrn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUrn, bIsDestroyed);
	DOREPLIFETIME(AUrn, TeamIndex);
	DOREPLIFETIME(AUrn, CurrentHealth);
}

float AUrn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDestroyed)
	{
		return 0.f;
	}

	CurrentHealth = FMath::Max(0.f, CurrentHealth - DamageAmount);
	OnRep_Health();

	if (CurrentHealth <= 0.f)
	{
		bIsDestroyed = true;
		OnUrnDestroyed(EventInstigator);
	}

	return DamageAmount;
}

void AUrn::OnRep_Health()
{
	OnHealthChanged(CurrentHealth);
}

void AUrn::OnUrnDestroyed_Implementation(AController* Destroyer) {}
void AUrn::OnHealthChanged_Implementation(float NewHealth) {}
