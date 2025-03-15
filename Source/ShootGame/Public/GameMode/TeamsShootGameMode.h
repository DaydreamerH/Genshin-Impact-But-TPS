// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/ShootGameMode.h"
#include "TeamsShootGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTGAME_API ATeamsShootGameMode : public AShootGameMode
{
	GENERATED_BODY()
public:
	ATeamsShootGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
	virtual void PlayerEliminated(APlayerCharacter* EliminatedCharacter, AMyPlayerController* VictimController, AMyPlayerController* AttackController) override;
protected:
	virtual void HandleMatchHasStarted() override;
	
};
