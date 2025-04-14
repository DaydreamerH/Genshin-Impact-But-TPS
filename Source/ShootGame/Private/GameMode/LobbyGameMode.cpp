// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootGame/Public/GameMode/LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"
#include "Player/MyPlayerState.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if(int32 numberOfPlayers =  GameState.Get()->PlayerArray.Num();
		numberOfPlayers == 2)
	{
		if(UWorld* world = GetWorld())
		{
			bUseSeamlessTravel = true;
			world->ServerTravel(FString("/Game/_Game/Maps/TestMap?listen"));
		}
	}
	if(AMyPlayerState* MyPlayerState = NewPlayer->GetPlayerState<AMyPlayerState>())
	{
		MyPlayerState->SetShowBarInOverheadWidget(false);
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


