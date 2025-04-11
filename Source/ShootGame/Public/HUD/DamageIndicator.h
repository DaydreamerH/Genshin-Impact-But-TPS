// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "DamageIndicator.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTGAME_API UDamageIndicator : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(meta=(BindWidget))
	UBorder* DamageBox;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* DamageIndicatorAnimation;

	UFUNCTION()
	void OnDamageAnimationFinished();
	
public:
	void RotateBox(float Angle);
	virtual void NativeConstruct() override;
};
