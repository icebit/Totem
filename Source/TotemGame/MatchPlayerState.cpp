#include "MatchPlayerState.h"
#include "CountdownComponent.h"
#include "Net/UnrealNetwork.h"

AMatchPlayerState::AMatchPlayerState()
{
	CountdownComponent = CreateDefaultSubobject<UCountdownComponent>(TEXT("CountdownComponent"));
	bIsReady = false;
}

void AMatchPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMatchPlayerState, bIsReady);
	DOREPLIFETIME(AMatchPlayerState, PlayerName);
}

void AMatchPlayerState::SetReady(bool bReady)
{
	bIsReady = bReady;
}

bool AMatchPlayerState::ServerReady_Validate() { return true; }
void AMatchPlayerState::ServerReady_Implementation()
{
	bIsReady = true;
}

bool AMatchPlayerState::ServerSetName_Validate(const FString& NewName) { return !NewName.IsEmpty(); }
void AMatchPlayerState::ServerSetName_Implementation(const FString& NewName)
{
	PlayerName = NewName;
	SetPlayerName(NewName);
}

void AMatchPlayerState::OnRep_Name()
{
	SetPlayerName(PlayerName);
}
