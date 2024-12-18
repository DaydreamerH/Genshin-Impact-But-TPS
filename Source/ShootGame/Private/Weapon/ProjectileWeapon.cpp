// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWeapon.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/BulletShell.h"
#include "Weapon/Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	if(FireAnimation && !WeaponMesh->IsPlaying())
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
		if(!HasAuthority()) return;
		APawn* InstigatorPawn = Cast<APawn>(GetOwner());

		if(const USkeletalMeshSocket* MuzzleFlashSocket = 
			GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash")))
		{
			FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
			FVector ToTarget = HitTarget - SocketTransform.GetLocation();
			FRotator TargetRotation = ToTarget.Rotation();
			if(ProjectileClass)
			{
				FActorSpawnParameters SpawnParameters;
				SpawnParameters.Owner = GetOwner();
				SpawnParameters.Instigator = InstigatorPawn;
				UWorld* World = GetWorld();
				if(World)
				{
					World->SpawnActor<AProjectile>(
						ProjectileClass,
						SocketTransform.GetLocation(),
						TargetRotation,
						SpawnParameters
					);
				}
			}
			if(BulletShellClass)
			{
				const USkeletalMeshSocket* AmmoEjectSocket = 
					WeaponMesh->GetSocketByName(FName("AmmoEject"));
				if(AmmoEjectSocket)
				{
					FTransform AmmoSocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
					if(UWorld* World = GetWorld())
					{
						World->SpawnActor<ABulletShell>(
							BulletShellClass,
							AmmoSocketTransform.GetLocation(),
							AmmoSocketTransform.GetRotation().Rotator()
							);
					}
			
				}
			}
		}
	}
	
}
