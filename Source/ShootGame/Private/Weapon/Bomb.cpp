// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Bomb.h"

ABomb::ABomb()
{
	GetWeaponMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
