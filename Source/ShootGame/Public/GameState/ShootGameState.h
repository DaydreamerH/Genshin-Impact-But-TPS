// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Player/MyPlayerState.h"
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

	UPROPERTY()
	TArray<AMyPlayerState*> RedTeam;
	UPROPERTY()
	TArray<AMyPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing=OnRep_RedTeamScore)
	float RedTeamScore = 0.f;
	UFUNCTION()
	void OnRep_RedTeamScore() const;
	
	UPROPERTY(ReplicatedUsing=OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;
	UFUNCTION()
	void OnRep_BlueTeamScore() const;

	void RedTeamScores(float Score = 1.f);
	void BlueTeamScores(float Score = 1.f);
	virtual void BeginPlay() override;
private:
	float TopScore = 0.f;
	UPROPERTY(EditAnywhere)
	USoundBase* BackgroundMusic;
};
