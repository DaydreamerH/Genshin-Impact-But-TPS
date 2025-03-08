// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/HitScanWeapon.h"
#include "ShotGunWeapon.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTGAME_API AShotGunWeapon : public AHitScanWeapon
{
	GENERATED_BODY()
public:
	// virtual void Fire(const FVector& HitTarget) override;
	virtual void FireShotGun(const TArray<FVector_NetQuantize>& HitTargets);
	void ShotGunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);
private:
	UPROPERTY(EditAnywhere, Category="Weapon Scatter")
	uint32 NumberOfPellets = 10;
	
};
