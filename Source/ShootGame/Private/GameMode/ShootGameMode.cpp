// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootGame/Public/GameMode/ShootGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "GameState/ShootGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MyPlayerState.h"
#include "Player/PlayerCharacter.h"
#include "PlayerController/MyPlayerController.h"
#include "Player/MyPlayerState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

void AShootGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();It;++It)
	{
		if(AMyPlayerController* PlayerController = Cast<AMyPlayerController>(*It))
		{
			PlayerController->OnMatchStateSet(MatchState);
		}
	}

	
}

void AShootGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AShootGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(MatchState == MatchState::WaitingToStart)
	{
		CountDownTime = WarmUpTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		
		if(CountDownTime<=0.f)
		{
			StartMatch();
		}
	}
	else if(MatchState == MatchState::InProgress)
	{
		CountDownTime = WarmUpTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(CountDownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if(MatchState == MatchState::Cooldown)
	{
		CountDownTime = CooldownTime + WarmUpTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(CountDownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

int AShootGameMode::GetPlayerIndex()
{
	return NextPlayerIndex++;
}

AShootGameMode::AShootGameMode()
{
	NextPlayerIndex = 0;
	bDelayedStart = true;
}

void AShootGameMode::PlayerEliminated(APlayerCharacter* EliminatedCharacter, AMyPlayerController* VictimController,
	AMyPlayerController* AttackController)
{
	AMyPlayerState* AttackerPlayerState = AttackController ? Cast<AMyPlayerState>(AttackController->PlayerState):nullptr;
	AMyPlayerState* VictimPlayerState = VictimController ? Cast<AMyPlayerState>(VictimController->PlayerState):nullptr;


	if(AShootGameState* ShootGameState = GetGameState<AShootGameState>();
		AttackerPlayerState && AttackerPlayerState!=VictimPlayerState
		&& ShootGameState)
	{
		AttackerPlayerState->AddToScore(1.f);
		ShootGameState->UpdateTopScore(AttackerPlayerState);
	}

	if(VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	
	if(EliminatedCharacter)
	{
		EliminatedCharacter->Elim(false);
	}

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if(AMyPlayerController* PlayerController = Cast<AMyPlayerController>(*It);
			PlayerController && AttackerPlayerState && VictimPlayerState)
		{
			PlayerController->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void AShootGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if(ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if(ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num()-1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

void AShootGameMode::PlayerLeftGame(AMyPlayerState* PlayerLeaving)
{
	if(AShootGameState* ShootGameState = GetGameState<AShootGameState>();
		ShootGameState && ShootGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		ShootGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	if(APlayerCharacter* CharacterLeaving =
		Cast<APlayerCharacter>(PlayerLeaving->GetPawn()))
	{
		CharacterLeaving->Elim(true);
	}
}
