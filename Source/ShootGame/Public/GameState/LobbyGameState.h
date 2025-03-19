// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "LobbyGameState.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTGAME_API ALobbyGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
private:
	UPROPERTY(EditAnywhere)
	USoundBase* LobbyMusic;

	void PlayBackGroundMusic();
};
