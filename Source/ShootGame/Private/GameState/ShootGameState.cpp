// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/ShootGameState.h"

#include "GameFramework/PlayerState.h"
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
		PlayerController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AShootGameState::OnRep_BlueTeamScore()
{
	if(AMyPlayerController* PlayerController
		= Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PlayerController->SetHUDBlueTeamScore(RedTeamScore);
	}
}

void AShootGameState::RedTeamScores()
{
	++RedTeamScore;

	if(AMyPlayerController* PlayerController
		= Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PlayerController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AShootGameState::BlueTeamScores()
{
	++BlueTeamScore;
	if(AMyPlayerController* PlayerController
		= Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PlayerController->SetHUDBlueTeamScore(RedTeamScore);
	}
}
