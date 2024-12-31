// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShootGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTGAME_API AShootGameMode : public AGameMode
{
	GENERATED_BODY()
private:
	int NextPlayerIndex;
protected:
	virtual void OnMatchStateSet() override;
public:
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	int GetPlayerIndex();
	AShootGameMode();
	virtual void PlayerEliminated
	(class APlayerCharacter* EliminatedCharacter,
		class AMyPlayerController* VictimController,
		class AMyPlayerController* AttackController);
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController);

	UPROPERTY(EditDefaultsOnly)
	float WarmUpTime = 10.f;

	float LevelStartingTime = 0.f;
private:
	float CountDownTime = 0.f;
};
