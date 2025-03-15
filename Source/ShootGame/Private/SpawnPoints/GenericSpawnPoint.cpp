#include "SpawnPoints/GenericSpawnPoint.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Weapon/Weapon.h"

AGenericSpawnPoint::AGenericSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AGenericSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	StartSpawnWeaponTimer(nullptr);
}

void AGenericSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGenericSpawnPoint::SpawnWeapon()
{
	if (WeaponClasses.Num() > 0)
	{
		int32 Selection = FMath::RandRange(0, WeaponClasses.Num() - 1);
		// 生成武器
		SpawnedWeapon = GetWorld()->SpawnActor<AWeapon>
			(WeaponClasses[Selection], GetActorLocation(), GetActorRotation());

		if (SpawnedWeapon)
		{
			SpawnedWeapon->OnWeaponStateChanged.AddDynamic(this, &ThisClass::OnWeaponStateChanged);
		}
	}
}

void AGenericSpawnPoint::OnWeaponStateChanged(EWeaponState WeaponState)
{
	if (SpawnedWeapon && WeaponState != EWeaponState::EWS_Initial)
	{
		SpawnedWeapon->OnWeaponStateChanged.RemoveDynamic(this, &AGenericSpawnPoint::OnWeaponStateChanged);
		SpawnedWeapon = nullptr;
		StartSpawnWeaponTimer(SpawnedWeapon);
	}
}

void AGenericSpawnPoint::StartSpawnWeaponTimer(AActor* DestroyedActor)
{
	GetWorld()->GetTimerManager().SetTimer
		(SpawnTimerHandle,
			this,
			&AGenericSpawnPoint::SpawnWeaponTimerFinished,
			SpawnInterval, false);
}

void AGenericSpawnPoint::SpawnWeaponTimerFinished()
{
	SpawnWeapon();
}
