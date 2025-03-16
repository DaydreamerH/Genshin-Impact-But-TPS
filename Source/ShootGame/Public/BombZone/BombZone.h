// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/team.h"
#include "BombZone.generated.h"


class UBoxComponent;

UCLASS()
class SHOOTGAME_API ABombZone : public AActor
{
	GENERATED_BODY()
	
public:	
	ABombZone();

protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY(EditAnywhere)
	UBoxComponent* ZoneSphere;

	UPROPERTY(EditAnywhere)
	ETeam Team;

	UPROPERTY(EditAnywhere)
	float Score = 20.f;
	
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
