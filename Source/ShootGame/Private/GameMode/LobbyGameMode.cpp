// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootGame/Public/GameMode/LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if(int32 numberOfPlayers =  GameState.Get()->PlayerArray.Num();
		numberOfPlayers == 6)
	{
		UWorld* world = GetWorld();
		if(world)
		{
			bUseSeamlessTravel = true;
			world->ServerTravel(FString("/Game/_Game/Maps/TeamMap?listen"));
		}
	}
}

int ALobbyGameMode::GetPlayerIndex()
{
	return NextPlayerIndex++;
}

ALobbyGameMode::ALobbyGameMode()
{
	NextPlayerIndex = 0;
}


