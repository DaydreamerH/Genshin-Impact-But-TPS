// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/TeamsShootGameMode.h"

#include "GameState/ShootGameState.h"
#include "Kismet/GameplayStatics.h"

void ATeamsShootGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if(AShootGameState* TGameState =
		Cast<AShootGameState>(UGameplayStatics::GetGameState(this)))
	{
		if(AMyPlayerState* PlayerState = NewPlayer->GetPlayerState<AMyPlayerState>();
				PlayerState && PlayerState->GetTeam() == ETeam::ET_NoTeam)
		{
			if(TGameState->BlueTeam.Num() < TGameState->RedTeam.Num())
			{
				TGameState->BlueTeam.AddUnique(PlayerState);
				PlayerState->SetTeam(ETeam::ET_BlueTeam);
			}
			else
			{
				TGameState->RedTeam.AddUnique(PlayerState);
				PlayerState->SetTeam(ETeam::ET_RedTeam);
			}
		}
	}
}

void ATeamsShootGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if(AShootGameState* TGameState =
		Cast<AShootGameState>(UGameplayStatics::GetGameState(this)))
	{
		if(AMyPlayerState* PlayerState = Exiting->GetPlayerState<AMyPlayerState>();
				PlayerState && PlayerState->GetTeam() == ETeam::ET_NoTeam)
		{
			if(TGameState->RedTeam.Contains(PlayerState))
			{
				TGameState->RedTeam.Remove(PlayerState);
			}
			else if(TGameState->BlueTeam.Contains(PlayerState))
			{
				TGameState->BlueTeam.Remove(PlayerState);
			}
			PlayerState->SetTeam(ETeam::ET_NoTeam);
		}
	}
}

void ATeamsShootGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	if(AShootGameState* TGameState =
		Cast<AShootGameState>(UGameplayStatics::GetGameState(this)))
	{
		for(auto PState : TGameState->PlayerArray)
		{
			if(AMyPlayerState* PlayerState = Cast<AMyPlayerState>(PState.Get());
				PlayerState && PlayerState->GetTeam() == ETeam::ET_NoTeam)
			{
				if(TGameState->BlueTeam.Num() < TGameState->RedTeam.Num())
				{
					TGameState->BlueTeam.AddUnique(PlayerState);
					PlayerState->SetTeam(ETeam::ET_BlueTeam);
				}
				else
				{
					TGameState->RedTeam.AddUnique(PlayerState);
					PlayerState->SetTeam(ETeam::ET_RedTeam);
				}
			}
		}
	}
}

float ATeamsShootGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	const AMyPlayerState* AttackerState = Attacker->GetPlayerState<AMyPlayerState>();
	const AMyPlayerState* VictimState = Victim->GetPlayerState<AMyPlayerState>();

	if(AttackerState == nullptr || VictimState == nullptr)return BaseDamage;
	
	if(VictimState == AttackerState)return BaseDamage;

	if(AttackerState->GetTeam() == VictimState->GetTeam())return 0.f;

	return BaseDamage;
}
