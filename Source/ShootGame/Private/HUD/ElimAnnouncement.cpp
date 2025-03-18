// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/ElimAnnouncement.h"

#include "Components/TextBlock.h"
#include "Player/PlayerCharacter.h"
#include "PlayerController/MyPlayerController.h"

void UElimAnnouncement::SetElimAnnouncementText(AMyPlayerState* Attacker, AMyPlayerState* Victim)
{
	if(MyTeam == ETeam::ET_NoTeam)
	{
		if(GetWorld())
		{
			if(AMyPlayerController* PlayerController =
				Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController()))
			{
				if(AMyPlayerState* PlayerState = PlayerController->GetPlayerState<AMyPlayerState>())
				{
					MyTeam = PlayerState->GetTeam();
				}
			}
		}
	}
	
	if(Victim)
	{
		VictimText->SetText(FText::FromString(Victim->GetPlayerName()));
		if(Victim->GetTeam() == MyTeam)
		{
			VictimText->SetColorAndOpacity(FriendColor);
		}
		else
		{
			VictimText->SetColorAndOpacity(EnemyColor);
		}
	}

	if(Attacker)
	{
		AttackerText->SetText(FText::FromString(Attacker->GetPlayerName()));
		if(Attacker->GetTeam() == MyTeam)
		{
			AttackerText->SetColorAndOpacity(FriendColor);
		}
		else
		{
			AttackerText->SetColorAndOpacity(EnemyColor);
		}
	}
	
}
