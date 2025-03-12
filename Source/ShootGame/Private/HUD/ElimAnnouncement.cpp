// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/ElimAnnouncement.h"

#include "Components/TextBlock.h"

void UElimAnnouncement::SetElimAnnouncementText(FString AttackerName, FString VictimName)
{
	const FString ElimAnnouncementText =
		FString::Printf(TEXT("%s %s %s"), *AttackerName, TEXT("击败"), *VictimName);
	if(AnnouncementText)
	{
		AnnouncementText->SetText(FText::FromString(ElimAnnouncementText));
	}
}
