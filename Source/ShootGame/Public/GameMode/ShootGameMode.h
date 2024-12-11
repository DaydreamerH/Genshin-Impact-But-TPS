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
public:
	int GetPlayerIndex();
	AShootGameMode();
};
