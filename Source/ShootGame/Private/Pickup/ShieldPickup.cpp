// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup/ShieldPickup.h"

#include "Components/BuffComponent.h"
#include "Player/PlayerCharacter.h"

void AShieldPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if(const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor))
	{
		if(const UBuffComponent* Buff = PlayerCharacter->GetBuff())
		{
			Buff->ReplenishShield(ShieldAmount);
		}
	}
	Destroy();
}
