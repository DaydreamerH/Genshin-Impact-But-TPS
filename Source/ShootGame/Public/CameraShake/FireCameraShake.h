// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Shakes/LegacyCameraShake.h"
#include "Weapon/Weapon.h"
#include "FireCameraShake.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTGAME_API UFireCameraShake : public ULegacyCameraShake
{
	GENERATED_BODY()

public:
	UFireCameraShake();
	void SetCameraShakeParams(FUCameraShakeParams& CameraShakeParams);
};
