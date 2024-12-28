// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/MyPlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/PlayerHUD.h"
#include "Player/PlayerCharacter.h"

void AMyPlayerController::SetHUDHealth(float Health, float MaxHealth) const
{
	PlayerHUD ==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->HealthBar &&PlayerHUD->CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health/MaxHealth;
		PlayerHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText =
			FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));

		PlayerHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void AMyPlayerController::ResetCrosshair() const
{
	if(PlayerHUD)
	{
		FHUDPackage HUDPackage;
		PlayerHUD->SetHUDPackage(HUDPackage);
	}
}

void AMyPlayerController::ResetHUD() const
{
	ResetCrosshair();
	if(const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetCharacter()))
	{
		const float MaxHealth = PlayerCharacter->GetMaxHealth();
		SetHUDHealth(MaxHealth, MaxHealth);
	}
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PlayerHUD = Cast<APlayerHUD>(GetHUD());
}

