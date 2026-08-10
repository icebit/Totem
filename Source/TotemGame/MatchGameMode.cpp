#include "MatchGameMode.h"
#include "MatchPlayerState.h"
#include "MatchGameState.h"
#include "MatchPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

AMatchGameMode::AMatchGameMode()
{
	RequiredNumPlayers = 2;
	CountdownDuration = 3.0f;
	MatchState = EMatchState::PendingStart;
	bAllReady = false;
}

void AMatchGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	CheckReady();
}

void AMatchGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	CheckReady();
}

void AMatchGameMode::Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	Super::Killed(Killer, KilledPlayer, KilledPawn, DamageType);

	NotifyTotemKilled(Killer, KilledPlayer, KilledPawn, DamageType);

	AMatchPlayerController* VictimController = Cast<AMatchPlayerController>(KilledPlayer);
	if (VictimController)
	{
		VictimController->ClientOnRespawn();
	}

	AMatchPlayerController* KillerController = Cast<AMatchPlayerController>(Killer);
	if (KillerController)
	{
		AMatchPlayerState* KillerPS = Cast<AMatchPlayerState>(Killer->PlayerState);
		AMatchPlayerState* VictimPS = Cast<AMatchPlayerState>(KilledPlayer ? KilledPlayer->PlayerState : nullptr);
		if (KillerPS && VictimPS)
		{
			KillerController->AddToKillfeed(
				KillerPS->GetPlayerName(),
				VictimPS->GetPlayerName(),
				Killer == KilledPlayer);
		}
	}
}

int32 AMatchGameMode::GetNumReadyPlayers()
{
	int32 ReadyCount = 0;
	for (APlayerState* PS : GameState->PlayerArray)
	{
		AMatchPlayerState* MatchPS = Cast<AMatchPlayerState>(PS);
		if (MatchPS && MatchPS->bIsReady)
		{
			ReadyCount++;
		}
	}
	return ReadyCount;
}

bool AMatchGameMode::AreAllPlayersReady()
{
	int32 PlayerCount = GameState->PlayerArray.Num();
	if (PlayerCount < RequiredNumPlayers)
	{
		return false;
	}
	return GetNumReadyPlayers() >= PlayerCount;
}

void AMatchGameMode::CheckReady()
{
	if (MatchState != EMatchState::PendingStart)
	{
		return;
	}

	if (AreAllPlayersReady())
	{
		BeginCountdown();
	}
}

void AMatchGameMode::BeginCountdown()
{
	SetMatchState(EMatchState::Countdown);

	AMatchGameState* MatchGS = GetGameState<AMatchGameState>();
	if (MatchGS && MatchGS->CountdownComponent)
	{
		MatchGS->CountdownComponent->InitialCountdown = static_cast<int32>(CountdownDuration);
		MatchGS->CountdownComponent->BeginCountdown();
	}

	GetWorldTimerManager().SetTimer(CountdownTimer, this, &AMatchGameMode::BeginMatch, CountdownDuration, false);
}

void AMatchGameMode::BeginMatch()
{
	SetMatchState(EMatchState::InProgress);
	OnStartMatch();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMatchPlayerController* PC = Cast<AMatchPlayerController>(It->Get());
		if (PC)
		{
			PC->ClientOnMatchStart();
		}
	}
}

void AMatchGameMode::EndMatch()
{
	SetMatchState(EMatchState::GameOver);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMatchPlayerController* PC = Cast<AMatchPlayerController>(It->Get());
		if (PC)
		{
			PC->ClientOnGameOver();
		}
	}
}

void AMatchGameMode::SetMatchState(EMatchState NewState)
{
	MatchState = NewState;
}

void AMatchGameMode::OnStartMatch_Implementation() {}
void AMatchGameMode::OnEndCountdown_Implementation() {}
void AMatchGameMode::NotifyTotemKilled_Implementation(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType) {}
