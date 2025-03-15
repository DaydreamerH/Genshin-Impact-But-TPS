// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Bomb.h"

#include "Components/SphereComponent.h"

ABomb::ABomb()
{
	GetWeaponMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABomb::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachmentTransformRules);
	OwnerPlayerCharacter=nullptr;
	OwnerPlayerController=nullptr;
	SetOwner(nullptr);
}

void ABomb::OnEquipped()
{
	ShowPickupWidget(false);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
}

void ABomb::OnDropped()
{
	if(HasAuthority())
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
}
