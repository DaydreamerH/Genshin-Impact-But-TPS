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
	if(const APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(GetOwner()))
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
			&& OwnerCharacter->IsLocallyControlled()
			&& HitCharacter)
		{
			OwnerCharacter->GetLagCompensation()->ServerProjectileScoreRequest(
				HitCharacter,
				TraceStart,
				InitialVelocity,
				OwnerPlayerController->GetServerTime() - OwnerPlayerController->SingleTripTime
			);
		}
	}
	
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();
	
}
