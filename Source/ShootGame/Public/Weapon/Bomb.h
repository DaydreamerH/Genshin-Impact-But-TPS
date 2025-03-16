// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Weapon.h"
#include "Bomb.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTGAME_API ABomb : public AWeapon
{
	GENERATED_BODY()
public:
	ABomb();
	virtual void Dropped() override;
protected:
	virtual void BeginPlay() override;
	virtual void OnEquipped() override;
	virtual void OnDropped() override;
private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* BombMesh;
};
