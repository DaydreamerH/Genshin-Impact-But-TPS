// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTGAME_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void ShowOverheadWidget();
	void SetDisplayText(const FString& TextToDisplay, bool bUseRedColor) const;
	void UpdateHealthBar(float Health, float MaxLife);
	void UpdateShieldBar(float Shield, float MaxShield);
	void UpdateBar(float Health, float MaxHealth, float Shield, float MaxShield);
	
private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;
	FSlateColor RedColor = FSlateColor(FLinearColor(1.0f, 0.390901f, 0.382375f, 1.0f));
	FSlateColor BlueColor = FSlateColor(FLinearColor(0.202818f,0.604446f,1.0f,1.0f));
	
protected:
	virtual void NativeDestruct() override;
};
