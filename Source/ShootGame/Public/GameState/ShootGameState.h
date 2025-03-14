// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ShootGameState.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTGAME_API AShootGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(APlayerState* ScoringPlayer);
	UPROPERTY(Replicated)
	TArray<class APlayerState*> TopScoringPlayers;

	TArray<AMyPlayerState*> RedTeam;
	TArray<AMyPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing=OnRep_RedTeamScore)
	float RedTeamScore = 0.f;
	UFUNCTION()
	void OnRep_RedTeamScore();
	
	UPROPERTY(ReplicatedUsing=OnRep_RedTeamScore)
	float BlueTeamScore = 0.f;
	UFUNCTION()
	void OnRep_BlueTeamScore();
	
private:
	float TopScore = 0.f;
};
