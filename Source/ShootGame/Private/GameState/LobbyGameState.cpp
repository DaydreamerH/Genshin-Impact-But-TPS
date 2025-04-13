// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/LobbyGameState.h"

#include "Kismet/GameplayStatics.h"

void ALobbyGameState::BeginPlay()
{
	Super::BeginPlay();
	
	PlayBackGroundMusic();
}

void ALobbyGameState::PlayBackGroundMusic()
{
	LobbyBgAudioComponent = UGameplayStatics::SpawnSound2D(this, LobbyMusic, 0.6);
}
