// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootGame/Public/GameMode/ShootGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerCharacter.h"
#include "PlayerController/MyPlayerController.h"

int AShootGameMode::GetPlayerIndex()
{
	return NextPlayerIndex++;
}

AShootGameMode::AShootGameMode()
{
	NextPlayerIndex = 0;
}

void AShootGameMode::PlayerEliminated(APlayerCharacter* EliminatedCharacter, AMyPlayerController* VictimController,
	AMyPlayerController* AttackController)
{
	if(EliminatedCharacter)
	{
		EliminatedCharacter->Elim();
	}
}

void AShootGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if(ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if(ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num()-1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}
