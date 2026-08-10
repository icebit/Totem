#include "UrnWarsGameMode.h"
#include "UrnWarsGameState.h"
#include "UrnWarsPlayerState.h"
#include "UrnWarsPlayerController.h"
#include "Urn.h"
#include "TotemPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

AUrnWarsGameMode::AUrnWarsGameMode()
{
	UrnRespawnDelay = 5.0f;
}

void AUrnWarsGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AUrnWarsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

bool AUrnWarsGameMode::CanDamage_Implementation(AController* Attacker, AController* Victim)
{
	return Super::CanDamage_Implementation(Attacker, Victim);
}

void AUrnWarsGameMode::Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	Super::Killed(Killer, KilledPlayer, KilledPawn, DamageType);

	if (Killer && Killer != KilledPlayer)
	{
		AUrnWarsPlayerState* KillerPS = Cast<AUrnWarsPlayerState>(Killer->PlayerState);
		if (KillerPS)
		{
			KillerPS->Souls += 1;
		}
	}
}

void AUrnWarsGameMode::OnUrnDestroyed(int32 DestroyedTeam, AController* Destroyer)
{
	NotifyUrnDestroyed(DestroyedTeam, Destroyer);
	CheckGameOver();
}

void AUrnWarsGameMode::NotifyUrnDestroyed(int32 DestroyedTeam, AController* Destroyer)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AUrnWarsPlayerController* PC = Cast<AUrnWarsPlayerController>(It->Get());
		if (PC)
		{
			PC->TeamUrnDeath(DestroyedTeam, Destroyer);
		}
	}
}

void AUrnWarsGameMode::DestroyAndRespawn(AController* Player)
{
	APawn* Pawn = Player ? Player->GetPawn() : nullptr;
	if (Pawn)
	{
		Pawn->Destroy();
	}

	FTimerHandle RespawnTimer;
	GetWorldTimerManager().SetTimer(RespawnTimer, [this, Player]()
	{
		if (Player)
		{
			RestartPlayer(Player);
		}
	}, UrnRespawnDelay, false);
}

AUrn* AUrnWarsGameMode::GetUrn(int32 TeamIndex)
{
	AUrnWarsGameState* GS = GetGameState<AUrnWarsGameState>();
	if (GS)
	{
		return GS->GetUrn(TeamIndex);
	}
	return nullptr;
}

void AUrnWarsGameMode::CheckGameOver()
{
	if (MatchState != EMatchState::InProgress)
	{
		return;
	}

	AUrnWarsGameState* GS = GetGameState<AUrnWarsGameState>();
	if (!GS)
	{
		return;
	}

	for (int32 i = 0; i < NumTeams; i++)
	{
		if (GS->IsUrnDestroyed(i))
		{
			int32 WinningTeam = -1;
			for (int32 j = 0; j < NumTeams; j++)
			{
				if (j != i && !GS->IsUrnDestroyed(j))
				{
					WinningTeam = j;
					break;
				}
			}

			OnGameOver(WinningTeam);
			EndMatch();
			return;
		}
	}
}

void AUrnWarsGameMode::OnGameOver_Implementation(int32 WinningTeam)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AUrnWarsPlayerController* PC = Cast<AUrnWarsPlayerController>(It->Get());
		if (PC)
		{
			PC->SetWinner(WinningTeam);
		}
	}
}
