// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/team.h"
#include "Player/MyPlayerState.h"
#include "ElimAnnouncement.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTGAME_API UElimAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetElimAnnouncementText(const AMyPlayerState* Attacker, const AMyPlayerState* Victim);
	
	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* AnnouncementBox;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AnnouncementText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AttackerText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* VictimText;
private:
	ETeam MyTeam = ETeam::ET_NoTeam;
	const FLinearColor FriendColor = FLinearColor(0.202818f, 0.604446f, 1.000000f, 1.000000f);
	const FLinearColor EnemyColor = FLinearColor(1.000000f,0.390901f,0.382375f,1.000000f);
};
