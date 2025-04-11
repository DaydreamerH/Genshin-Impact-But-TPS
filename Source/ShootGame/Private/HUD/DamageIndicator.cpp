// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/DamageIndicator.h"

void UDamageIndicator::OnDamageAnimationFinished()
{
	RemoveFromParent();
}

void UDamageIndicator::NativeConstruct()
{
	Super::NativeConstruct();

	if(DamageIndicatorAnimation)
	{
		PlayAnimation(DamageIndicatorAnimation);

		FWidgetAnimationDynamicEvent EndDelegate;
		EndDelegate.BindDynamic(this, &ThisClass::OnDamageAnimationFinished);
		BindToAnimationFinished(DamageIndicatorAnimation, EndDelegate);
	}
}

void UDamageIndicator::RotateBox(float Angle)
{
	DamageBox->SetRenderTransformAngle(Angle);
}
