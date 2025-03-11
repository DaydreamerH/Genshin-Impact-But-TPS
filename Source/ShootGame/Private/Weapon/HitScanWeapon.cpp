// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"

#include "Components/LagCompensationComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Player/PlayerCharacter.h"
#include "PlayerController/MyPlayerController.h"
#include "Sound/SoundCue.h"
#include "Weapon/BulletShell.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	if(FireAnimation && !WeaponMesh->IsPlaying())
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
		if(BulletShellClass)
		{
			if(const USkeletalMeshSocket* AmmoEjectSocket = 
				WeaponMesh->GetSocketByName(FName("AmmoEject")))
			{
				FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
				if(UWorld* World = GetWorld())
				{
					World->SpawnActor<ABulletShell>(
						BulletShellClass,
						SocketTransform.GetLocation(),
						SocketTransform.GetRotation().Rotator()
						);
				}
			
			}
		}
		SpendRound();
		APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if(OwnerPawn==nullptr)return;
		AController* InstigatorController = OwnerPawn->GetController();

		const USkeletalMeshSocket*
			MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
		if(MuzzleFlashSocket)
		{
			const UWorld* World = GetWorld();
			FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform((GetWeaponMesh()));
			FVector Start = SocketTransform.GetLocation();

			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);
			
			if(APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(FireHit.GetActor()))
			{
				if(InstigatorController)
				{
					if(HasAuthority() && (OwnerPawn->IsLocallyControlled()||bUseServerSideRewind))
					{
						UGameplayStatics::ApplyDamage(
							PlayerCharacter,
							Damage,
							InstigatorController,
							this,
							UDamageType::StaticClass()
						);
					}
					else if(!HasAuthority() && bUseServerSideRewind)
					{
						OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ?
							Cast<APlayerCharacter>(OwnerPawn) : OwnerPlayerCharacter;
						OwnerPlayerController = OwnerPlayerController == nullptr ?
							Cast<AMyPlayerController>(InstigatorController) : OwnerPlayerController;

						if(OwnerPlayerCharacter
							&& OwnerPlayerController
							&& OwnerPlayerCharacter->GetLagCompensation()
							&& OwnerPlayerCharacter->IsLocallyControlled())
						{
							OwnerPlayerCharacter->GetLagCompensation()->ServerScoreRequest(
								PlayerCharacter,
								Start,
								HitTarget,
								OwnerPlayerController->GetServerTime()-OwnerPlayerController->SingleTripTime,
								this
							);
						}
					}
				}
				if(ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						ImpactParticles,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
				}
				if(HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						this,
						HitSound,
						FireHit.ImpactPoint
					);
				}
			}
			
			if(MuzzleFlash)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					World,
					MuzzleFlash,
					SocketTransform.GetLocation()
				);
			}
			if(FireSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					FireSound,
					GetActorLocation()
				);
			}
		}
	}
	
}


void AHitScanWeapon::WeaponTraceHit(const FVector & TraceStart, const FVector & HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if(World)
	{
		FVector End = TraceStart + (HitTarget-TraceStart)*1.25f;
		
		World->LineTraceSingleByChannel(
					OutHit,
					TraceStart,
					End,
					ECC_Visibility
				);
		FVector BeamEnd;
		if(OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}

		DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Orange, true);
		
		if(BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if(Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}
