// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CharacterOverlay.h"

void UCharacterOverlay::SetHealthBar(float Percent)
{
	UFunction* Func = HealthBar->FindFunction(FName("UpdatePercent"));

	if(!Func)return;
	struct FUpdatePercentParams
	{
		double Percent;
	};

	FUpdatePercentParams Params;
	Params.Percent = Percent;
	
	HealthBar->ProcessEvent(Func, &Params);
}

void UCharacterOverlay::SetShieldBar(float Percent)
{
	UFunction* Func = ShieldBar->FindFunction(FName("UpdatePercent"));

	if(!Func)return;
	struct FUpdatePercentParams
	{
		double Percent;
	};

	FUpdatePercentParams Params;
	Params.Percent = Percent;
	
	ShieldBar->ProcessEvent(Func, &Params);
}

void UCharacterOverlay::SetCombatBar(float Percent)
{
	UFunction* Func = CombatBar->FindFunction(FName("UpdatePercent"));

	if(!Func)return;
	struct FUpdatePercentParams
	{
		double Percent;
	};

	FUpdatePercentParams Params;
	Params.Percent = Percent;
	
	CombatBar->ProcessEvent(Func, &Params);
}
