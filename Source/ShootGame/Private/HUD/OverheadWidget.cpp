#include "HUD/OverheadWidget.h"

#include "Components/Border.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UOverheadWidget::InitOverheadWidget(const FString& TextToDisplay, bool bUseRedColor) const
{
	const FLinearColor Color = bUseRedColor
		? FLinearColor(1.0f, 0.390901f, 0.382375f, 1.0f)
		: FLinearColor(0.202818f,0.604446f,1.0f,1.0f);
	if(DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
		DisplayText->SetColorAndOpacity(Color);
	}
	if(LifeBorder)
	{
		LifeBorder->SetBrushColor(Color);
	}
	if(ShieldBorder)
	{
		ShieldBorder->SetBrushColor(Color);
	}
	if(HealthBar)
	{
		FProgressBarStyle BarStyle = HealthBar->GetWidgetStyle();
		BarStyle.BackgroundImage.TintColor = Color;
		HealthBar->SetWidgetStyle(BarStyle);
	}
	if(ShieldBar)
	{
		FProgressBarStyle BarStyle = ShieldBar->GetWidgetStyle();
		BarStyle.BackgroundImage.TintColor = Color;
		ShieldBar->SetWidgetStyle(BarStyle);
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
