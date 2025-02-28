// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Player/PlayerCharacter.h"
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
		SpendRounnd();
		const APawn* OnwerPawn = Cast<APawn>(GetOwner());
		if(OnwerPawn==nullptr)return;
		AController* InstigatorController = OnwerPawn->GetController();

		const USkeletalMeshSocket*
			MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
		if(MuzzleFlashSocket)
		{
			const UWorld* World = GetWorld();
			FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform((GetWeaponMesh()));
			FVector Start = SocketTransform.GetLocation();
			FVector End = Start + (HitTarget - Start)*1.25f; // 在命中目标的后面一部分

			FHitResult FireHit;
			if(World)
			{
				World->LineTraceSingleByChannel(
					FireHit,
					Start,
					End,
					ECC_Visibility
				);
				FVector BeamEnd = End;
				if(FireHit.bBlockingHit)
				{
					BeamEnd = FireHit.ImpactPoint;
					if(APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(FireHit.GetActor()))
					{
						if(HasAuthority() && InstigatorController)
						{
							UGameplayStatics::ApplyDamage(
								PlayerCharacter,
								Damage,
								InstigatorController,
								this,
								UDamageType::StaticClass()
							);
						}
					}
					if(ImpactParticles)
					{
						UGameplayStatics::SpawnEmitterAtLocation(
							World,
							ImpactParticles,
							End,
							FireHit.ImpactNormal.Rotation()
						);
					}
				}
				if(BeamParticles)
				{
					if(UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
						World,
						BeamParticles,
						SocketTransform.GetLocation()
					))
					{
						Beam->SetVectorParameter(FName("Target"), BeamEnd);
					}
				}
			}
		
		}
	}
	
}
