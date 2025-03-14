// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/ShootGameState.h"

#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

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
}

void AShootGameState::OnRep_BlueTeamScore()
{
}
