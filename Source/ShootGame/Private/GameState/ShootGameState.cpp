// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/ShootGameState.h"

#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/MyPlayerController.h"

void AShootGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShootGameState, TopScoringPlayers);
	DOREPLIFETIME(AShootGameState, RedTeamScore);
	DOREPLIFETIME(AShootGameState, BlueTeamScore);
}

void AShootGameState::UpdateTopScore(APlayerState* ScoringPlayer)
{
	if(TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if(ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if(ScoringPlayer->GetScore()>TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void AShootGameState::OnRep_RedTeamScore()
{
	if(AMyPlayerController* PlayerController
		= Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		if(AMyPlayerState* MyPlayerState = PlayerController->GetPlayerState<AMyPlayerState>())
		{
			if(MyPlayerState->GetTeam() == ETeam::ET_RedTeam)
			{
				PlayerController->SetHUDMyTeamScore(RedTeamScore);
			}
			else if(MyPlayerState->GetTeam() == ETeam::ET_BlueTeam)
			{
				PlayerController->SetHUDEnemyTeamScore(RedTeamScore);
			}
		}
	}
}

void AShootGameState::OnRep_BlueTeamScore()
{
	if(AMyPlayerController* PlayerController
		= Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		if(AMyPlayerState* MyPlayerState = PlayerController->GetPlayerState<AMyPlayerState>())
		{
			if(MyPlayerState->GetTeam() == ETeam::ET_RedTeam)
			{
				PlayerController->SetHUDEnemyTeamScore(BlueTeamScore);
			}
			else if(MyPlayerState->GetTeam() == ETeam::ET_BlueTeam)
			{
				PlayerController->SetHUDMyTeamScore(BlueTeamScore);
			}
		}
	}
}

void AShootGameState::RedTeamScores(const float Score)
{
	RedTeamScore += Score;
	RedTeamScore = FMath::Clamp(RedTeamScore, 0, RedTeamScore);

	if(AMyPlayerController* PlayerController
		= Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		if(AMyPlayerState* MyPlayerState = PlayerController->GetPlayerState<AMyPlayerState>())
		{
			if(MyPlayerState->GetTeam() == ETeam::ET_RedTeam)
			{
				PlayerController->SetHUDMyTeamScore(RedTeamScore);
			}
			else if(MyPlayerState->GetTeam() == ETeam::ET_BlueTeam)
			{
				PlayerController->SetHUDEnemyTeamScore(RedTeamScore);
			}
		}
	}
}

void AShootGameState::BlueTeamScores(const float Score)
{
	BlueTeamScore += Score;
	BlueTeamScore = FMath::Clamp(BlueTeamScore, 0, BlueTeamScore);
	
	if(AMyPlayerController* PlayerController
		= Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		if(AMyPlayerState* MyPlayerState = PlayerController->GetPlayerState<AMyPlayerState>())
		{
			if(MyPlayerState->GetTeam() == ETeam::ET_RedTeam)
			{
				PlayerController->SetHUDEnemyTeamScore(BlueTeamScore);
			}
			else if(MyPlayerState->GetTeam() == ETeam::ET_BlueTeam)
			{
				PlayerController->SetHUDMyTeamScore(BlueTeamScore);
			}
		}
	}
}

void AShootGameState::BeginPlay()
{
	Super::BeginPlay();
	UGameplayStatics::PlaySound2D(this, BackgroundMusic, 0.5);
}
