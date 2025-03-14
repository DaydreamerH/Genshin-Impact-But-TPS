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
		SpendRound();
		APawn* InstigatorPawn = Cast<APawn>(GetOwner());
		UWorld* World = GetWorld();
		if(const USkeletalMeshSocket* MuzzleFlashSocket = 
			GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
			MuzzleFlashSocket && World)
		{
			FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
			FVector ToTarget = HitTarget - SocketTransform.GetLocation();
			FRotator TargetRotation = ToTarget.Rotation();
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = GetOwner();
			SpawnParameters.Instigator = InstigatorPawn;

			AProjectile* SpawnedProjectile;
			if(bUseServerSideRewind)
			{
				if(InstigatorPawn->HasAuthority())
				{
					if(InstigatorPawn->IsLocallyControlled())
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(
							ProjectileClass,
							SocketTransform.GetLocation(),
							TargetRotation,
							SpawnParameters);
						SpawnedProjectile->bUseServerSideRewind = false;
						SpawnedProjectile->Damage = Damage;
						SpawnedProjectile->HeadShotDamage = HeadShotDamage;
					}
					else
					{	
						SpawnedProjectile = World->SpawnActor<AProjectile>(
							SeverSideRewindProjectileClass,
							SocketTransform.GetLocation(),
							TargetRotation,
							SpawnParameters);
						SpawnedProjectile->bUseServerSideRewind = true;
					}
				}
				else
				{
					if(InstigatorPawn->IsLocallyControlled())
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(
							SeverSideRewindProjectileClass,
							SocketTransform.GetLocation(),
							TargetRotation,
							SpawnParameters);
						SpawnedProjectile->bUseServerSideRewind = true;
						SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
						SpawnedProjectile->InitialVelocity
							= SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
						SpawnedProjectile->Damage = Damage;
						SpawnedProjectile->HeadShotDamage = HeadShotDamage;
					}
					else
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(
							SeverSideRewindProjectileClass,
							SocketTransform.GetLocation(),
							TargetRotation,
							SpawnParameters);
						SpawnedProjectile->bUseServerSideRewind = false;
					}
				}
			}
			else
			{
				if(InstigatorPawn->HasAuthority())
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(
							ProjectileClass,
							SocketTransform.GetLocation(),
							TargetRotation,
							SpawnParameters);
					SpawnedProjectile->bUseServerSideRewind = false;
					SpawnedProjectile->Damage = Damage;
				}
			}
			if(BulletShellClass)
			{
				const USkeletalMeshSocket* AmmoEjectSocket = 
					WeaponMesh->GetSocketByName(FName("AmmoEject"));
				if(AmmoEjectSocket)
				{
					FTransform AmmoSocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
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
