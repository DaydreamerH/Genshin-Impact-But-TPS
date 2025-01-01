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
	UPROPERTY(Replicated)
	TArray<class APlayerState*> TopScoringPlayers;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(APlayerState* ScoringPlayer);
private:
	float TopScore = 0.f;
};
