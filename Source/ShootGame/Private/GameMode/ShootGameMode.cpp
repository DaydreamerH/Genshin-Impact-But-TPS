// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootGame/Public/GameMode/ShootGameMode.h"

int AShootGameMode::GetPlayerIndex()
{
	return NextPlayerIndex++;
}

AShootGameMode::AShootGameMode()
{
	NextPlayerIndex = 0;
}
