// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup/SpeedPickup.h"

#include "Components/BuffComponent.h"
#include "Player/PlayerCharacter.h"

void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResule)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResule);

	if(const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor))
	{
		if(UBuffComponent* Buff = PlayerCharacter->GetBuff())
		{
			Buff->BuffSpeed(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
		}
	}
	Destroy();
}
