// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ShotGunWeapon.h"

#include "Components/CombatComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Player/PlayerCharacter.h"
#include "Sound/SoundCue.h"
#include "Weapon/BulletShell.h"
/*
void AShotGunWeapon::Fire(const FVector& HitTarget)
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

			TMap<APlayerCharacter*, uint32> HitMap;
			for(uint32 i = 0;i<NumberOfPellets;i++)
			{
				FHitResult FireHit;
				WeaponTraceHit(Start, HitTarget, FireHit);
				if(APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(FireHit.GetActor()))
				{
					if(HasAuthority() && InstigatorController)
					{
						if(HitMap.Contains(PlayerCharacter))
						{
							HitMap[PlayerCharacter]++;
						}
						else
						{
							HitMap.Emplace(PlayerCharacter, 1);
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
						FireHit.ImpactPoint,
						.5f,
						FMath::FRandRange(-.5, .5f)
					);
				}
			}
			
			for(auto HitPair:HitMap)
			{
				if(InstigatorController && HitPair.Key && HasAuthority())
				{
					UGameplayStatics::ApplyDamage(
						HitPair.Key,
						Damage * HitPair.Value,
						InstigatorController,
						this,
						UDamageType::StaticClass()
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
*/

void AShotGunWeapon::FireShotGun(const TArray<FVector_NetQuantize>& HitTargets)
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

		if(const USkeletalMeshSocket*
			MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash"))
		{
			const UWorld* World = GetWorld();
			const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform((GetWeaponMesh()));
			const FVector Start = SocketTransform.GetLocation();

			TMap<APlayerCharacter*, uint32> HitMap;
			for(auto HitTarget : HitTargets)
			{
				FHitResult FireHit;
				WeaponTraceHit(Start, HitTarget, FireHit);
				if(APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(FireHit.GetActor()))
				{
					if(HitMap.Contains(PlayerCharacter))
					{
						HitMap[PlayerCharacter]++;
					}
					else
					{
						HitMap.Emplace(PlayerCharacter, 1);
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
						FireHit.ImpactPoint,
						.5f,
						FMath::FRandRange(-.5, .5f)
					);
				}
			}
			
			for(auto HitPair:HitMap)
			{
				if(InstigatorController && HitPair.Key && HasAuthority())
				{
					UGameplayStatics::ApplyDamage(
						HitPair.Key,
						Damage * HitPair.Value,
						InstigatorController,
						this,
						UDamageType::StaticClass()
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

void AShotGunWeapon::ShotGunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket*
			MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket == nullptr)
	{
		return;
	}
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform((GetWeaponMesh()));
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized*DistanceToSphere;

	
	for(uint32 i = 0;i<NumberOfPellets;i++)
	{
		FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		HitTargets.Add(TraceStart + ToEndLoc * TRACE_LENGTH);
	}
}
