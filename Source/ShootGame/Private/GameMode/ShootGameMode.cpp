// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootGame/Public/GameMode/ShootGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MyPlayerState.h"
#include "Player/PlayerCharacter.h"
#include "PlayerController/MyPlayerController.h"

void AShootGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();It;It++)
	{
		AMyPlayerController* PlayerController = Cast<AMyPlayerController>(*It);
		if(PlayerController)
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

	if(AttackerPlayerState && AttackerPlayerState!=VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}

	if(VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	
	if(EliminatedCharacter)
	{
		EliminatedCharacter->Elim();
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
