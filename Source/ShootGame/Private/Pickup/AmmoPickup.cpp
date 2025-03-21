// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup/AmmoPickup.h"

#include "Components/CombatComponent.h"
#include "Player/PlayerCharacter.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if(APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor))
	{
		if(UCombatComponent* Combat = PlayerCharacter->GetCombat())
		{
			Combat->PickupAmmo(WeaponType, AmmoAmount);
		}
	}
	Destroy();
}
