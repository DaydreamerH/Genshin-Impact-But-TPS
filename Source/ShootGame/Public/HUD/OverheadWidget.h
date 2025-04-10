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
	void InitOverheadWidget(const FString& TextToDisplay, bool bUseRedColor) const;
	void UpdateHealthBar(float Health, float MaxLife);
	void UpdateShieldBar(float Shield, float MaxShield);
	void UpdateBar(float Health, float MaxHealth, float Shield, float MaxShield);
	void HideBar();
	
private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;
	UPROPERTY(meta = (BindWidget))
	class UBorder* LifeBorder;
	UPROPERTY(meta = (BindWidget))
	UBorder* ShieldBorder;
protected:
	virtual void NativeDestruct() override;
};
