// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OverheadWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UOverheadWidget::SetDisplayText(const FString& TextToDisplay, bool bUseRedColor) const
{
	if(DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
		const FSlateColor Color = bUseRedColor ? RedColor : BlueColor;
		DisplayText->SetColorAndOpacity(Color);
	}
}

void UOverheadWidget::UpdateHealthBar(float Health, float MaxHealth)
{
	if(HealthBar)
	{
		HealthBar->SetPercent(Health/MaxHealth);
	}
}

void UOverheadWidget::UpdateShieldBar(float Shield, float MaxShield)
{
	if(ShieldBar)
	{
		ShieldBar->SetPercent(Shield/MaxShield);
	}
}

void UOverheadWidget::UpdateBar(float Health, float MaxHealth, float Shield, float MaxShield)
{
	UpdateHealthBar(Health, MaxHealth);
	UpdateShieldBar(Shield, MaxShield);
}

void UOverheadWidget::NativeDestruct()
{
	Super::NativeDestruct();
}
