// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Player/MyPlayerState.h"
#include "ShootGameMode.generated.h"

namespace MatchState
{
	extern SHOOTGAME_API const FName Cooldown; // 有人赢了，游戏结束，休息
}

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
	void PlayerLeftGame(AMyPlayerState* PlayerLeaving);
	UPROPERTY(EditDefaultsOnly)
	float WarmUpTime = 10.f;
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;
	float LevelStartingTime = 0.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	FORCEINLINE float GetCountdownTime() const {return CountDownTime;}

	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);
	
private:
	float CountDownTime = 0.f;
	
};
