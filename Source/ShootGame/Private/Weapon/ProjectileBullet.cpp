// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileBullet.h"

#include "Components/LagCompensationComponent.h"
#include "Kismet/GameplayStatics.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Player/PlayerCharacter.h"
#include "PlayerController/MyPlayerController.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if(const FName PropertyName = PropertyChangedEvent.Property == nullptr
		? PropertyChangedEvent.Property->GetFName() : NAME_None;
		PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))
	{
		if(ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}

#endif

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	if(APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(GetOwner()))
	{
		AController* OwnerController = OwnerCharacter->GetController();
		if(OwnerCharacter->HasAuthority() && !bUseServerSideRewind)
		{
			const float DamageToCause = Hit.BoneName.ToString() == FString(TEXT("頭")) ? HeadShotDamage : Damage;
			UGameplayStatics::ApplyDamage
				(OtherActor, DamageToCause, OwnerController, this, UDamageType::StaticClass());
			Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
			return;
		}
		APlayerCharacter* HitCharacter = Cast<APlayerCharacter>(OtherActor);
		AMyPlayerController* OwnerPlayerController = Cast<AMyPlayerController>(OwnerCharacter->GetOwner());
		if(bUseServerSideRewind
			&& OwnerCharacter->GetLagCompensation()
			&& OwnerCharacter->IsLocallyControlled())
		{
			OwnerCharacter->GetLagCompensation()->ServerProjectileScoreRequest(
				HitCharacter,
				TraceStart,
				InitialVelocity,
				OwnerPlayerController->GetServerTime() - OwnerPlayerController->SingleTripTime,
				this
			);
		}
	}
	
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	/*
	FPredictProjectilePathParams Params;
	Params.bTraceWithChannel = true;
	Params.bTraceWithCollision = true;
	Params.DrawDebugTime = 5.f;
	Params.DrawDebugType = EDrawDebugTrace::ForDuration;
	Params.LaunchVelocity = GetActorForwardVector()*InitialSpeed;
	Params.MaxSimTime = 4.f;
	Params.ProjectileRadius = 5.f;
	Params.SimFrequency = 30.f;
	Params.StartLocation = GetActorLocation();
	Params.TraceChannel = ECC_Visibility;
	Params.ActorsToIgnore.Add(this);
	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, Params, PathResult);*/
	
}
