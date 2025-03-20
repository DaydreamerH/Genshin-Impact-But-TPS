

#include "Components/CombatComponent.h"

#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/PlayerHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerCharacter.h"
#include "PlayerController/MyPlayerController.h"
#include "Sound/SoundCue.h"
#include "Weapon/ShotGunWeapon.h"
#include "Weapon/Weapon.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	EquippedWeapon = nullptr;
	bAiming = false;
	Character = nullptr;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if(Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		if(Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}


void UCombatComponent::SetAiming(bool bIsAiming)
{
	if(Character == nullptr || EquippedWeapon == nullptr)return;
	
	bAiming = bIsAiming;
	EquippedWeapon->SwitchAim(bAiming);
	ServerSetAiming(bIsAiming);
	Character->GetCharacterMovement()->MaxWalkSpeed
		= bIsAiming ? AimWalkSpeed:Character->GetCharacterMovement()->MaxWalkSpeed;

	if(Character->IsLocallyControlled()
		&& EquippedWeapon->GetWeaponType()==EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
	if(Character->IsLocallyControlled())
	{
		bAimButtonPressed = bIsAiming;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		PlayEquipWeaponSound(EquippedWeapon);
		EquippedWeapon->SetHUDAmmo();
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if(SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToBackpack(SecondaryWeapon);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		PlayEquipWeaponSound(SecondaryWeapon);
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if(Character==nullptr)return;
	if(bFireButtonPressed)
	{
		if(CanFire())
		{
			switch (EquippedWeapon->FireType)
			{
				case EFireType::EFT_Projectile:
					FireProjectileWeapon();
					break;
				case EFireType::EFT_HitScan:
					FireHitScanWeapon();
					break;
				case EFireType::EFT_ShotGun:
					FireShotGun();
					break;
				default:
						break;
			}
		}
		else if(bPlayNoAmmoSound && CombatState == ECombatState::ECS_Unoccupied)
		{
			PlayNoAmmoSound();
		}
	}
	else
	{
		bPlayNoAmmoSound = true;
		if(EquippedWeapon && !EquippedWeapon->GetAutoFire())
		{
			ServerCooldown();
		}
	}
}

void UCombatComponent::ShotGunShellReload()
{
	if(Character && Character->HasAuthority())
	{
		if(UWorld* World = GetWorld())
		{
			float CurrentTime = World->GetTimeSeconds();
			if(CurrentTime - ShotGunLastReloadTime < ShotGunReloadTimeSpace)
			{
				return;
			}
			ShotGunLastReloadTime = CurrentTime;
		}
		
		UpdateShotGunAmmoValue();
	}
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if(CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);

		UpdateCarriedAmmo();
	}
}

void UCombatComponent::DropBomb()
{
	if(EquippedBomb)
	{
		bHoldingBomb = false;
		EquippedBomb->Dropped();
		Character->UnCrouch();
		if(EquippedWeapon!=nullptr)
		{
			Character->GetCharacterMovement()->bOrientRotationToMovement = false;
			Character->bUseControllerRotationYaw = true;
		}
	}
}

void UCombatComponent::ServerCooldown_Implementation()
{
	if(CombatState == ECombatState::ECS_Cooling)
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	Character->PlayFireMontage(bAiming);
	Character->SetCrosshairShootingFactor();
	EquippedWeapon->Fire(TraceHitTarget);
}

void UCombatComponent::ShotGunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTarget)
{
	AShotGunWeapon* ShotGunWeapon = Cast<AShotGunWeapon>(EquippedWeapon);
	if(ShotGunWeapon == nullptr || Character == nullptr)return;
	if(CombatState == ECombatState::ECS_Reloading
		|| CombatState == ECombatState::ECS_Unoccupied)
	{
		ShotGunWeapon->FireShotGun(TraceHitTarget);
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

void UCombatComponent::FireProjectileWeapon()
{
	// 我曾经在开火前再次更新射线检测的目标，不知道去掉会怎么样
	// FHitResult HitResult;
	// TraceUnderCrosshairs(HitResult);
	if(EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter?
			EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if(!Character->HasAuthority())
		{
			LocalFire(HitTarget);
		}
		ServerFire(HitTarget);	
	}
	
}

void UCombatComponent::FireHitScanWeapon()
{
	if(EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter?
			EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if(!Character->HasAuthority())
		{
			LocalFire(HitTarget);
		}
		ServerFire(HitTarget);	
	}
}

void UCombatComponent::FireShotGun()
{
	if(AShotGunWeapon* ShotGunWeapon = Cast<AShotGunWeapon>(EquippedWeapon);
		ShotGunWeapon && Character)
	{
		TArray<FVector_NetQuantize>HitTargets;
		ShotGunWeapon->ShotGunTraceEndWithScatter(HitTarget, HitTargets);
		if(!Character->HasAuthority())
		{
			ShotGunLocalFire(HitTargets);
		}
		ServerShotGunFire(HitTargets);
	}
}

void UCombatComponent::ServerShotGunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	MulticastShotGunFire(TraceHitTargets);
}

void UCombatComponent::MulticastShotGunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if(Character == nullptr || (Character->IsLocallyControlled() && !Character->HasAuthority()))
	{
		return;
	}
	
	ShotGunLocalFire(TraceHitTargets);
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2d ViewPortSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
	}

	FVector2d CrossHairLocation(ViewPortSize.X/2, ViewPortSize.Y/2);
	FVector CrossHairWorldPosition;
	FVector CrossHairWoldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrossHairLocation,
		CrossHairWorldPosition,
		CrossHairWoldDirection
	);
	if(bScreenToWorld)
	{
		FVector Start = CrossHairWorldPosition;

		if(Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrossHairWoldDirection*(DistanceToCharacter+100.f);
		}
		
		FVector End = Start + CrossHairWoldDirection * TRACE_LENGTH;
		
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECC_Visibility
		);

		if(!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if(EquippedWeapon == nullptr)return;
	if(Character == nullptr || Character->Controller == nullptr || Character->GetElimmed())return;
	Controller = Controller == nullptr?Cast<AMyPlayerController>(Character->Controller):Controller;

	if(Controller)
	{
		HUD = HUD == nullptr? Cast<APlayerHUD>(Controller->GetHUD()):HUD;
		if (HUD)
		{
			FHUDPackage HUDPackage ;
			if(EquippedWeapon == nullptr)
			{
				HUD->SetHUDPackage(HUDPackage);
				return;
			}
			HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
			HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
			HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
			HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;


			FVector2d WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2d VelocityMultiplierRange(0.f,1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;
			CrosshairVelocityFactor =
				FMath::GetMappedRangeValueClamped(
					WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if(bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 20.f);
			HUDPackage.CrosshairSpread =
				0.5f + CrosshairVelocityFactor - CrosshairAimFactor + CrosshairShootingFactor;
			
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::HandleReload()
{
	if(Character)
	{
		Character->PlayReloadMontage();
	}
}

int32 UCombatComponent::AmountToReload()
{
	if(EquippedWeapon == nullptr)return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapcity() - EquippedWeapon->GetAmmo();
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(AmountCarried,RoomInMag);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}

void UCombatComponent::TossGrenade()
{
	if(CombatState != ECombatState::ECS_Unoccupied
		|| EquippedWeapon == nullptr
		|| Grenades <= 0 ) return;
	CombatState = ECombatState::ECS_TossGrenade;
	if(Character)
	{
		Character->PlayTossGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowGrenade(true);
	}
	if(Character && !Character->HasAuthority())
	{
		ServerTossGrenade();
	}
	else if(Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGernades();
	}
}

void UCombatComponent::DropEquippedWeapon()
{
	if(EquippedWeapon!=nullptr)
	{
		EquippedWeapon->Dropped();
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach) const
{
	if(ActorToAttach == nullptr
		|| Character == nullptr
		|| Character->GetMesh() == nullptr)return;
	
	if(const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket")))
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if(ActorToAttach == nullptr
		|| Character == nullptr
		|| Character->GetMesh() == nullptr)return;
	
	if(const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket")))
	{
		BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachBombToLeftHand(AWeapon* Bomb)
{
	if(Bomb == nullptr
		|| Character == nullptr
		|| Character->GetMesh() == nullptr)return;
	
	if(const USkeletalMeshSocket* BombSocket
		= Character->GetMesh()->GetSocketByName(FName("LeftBombSocket")))
	{
		BombSocket->AttachActor(Bomb, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if(ActorToAttach == nullptr
		|| Character == nullptr
		|| Character->GetMesh() == nullptr)return;
	
	if(const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("LeftHandSocket")))
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::ShowGrenade(bool bShowGrenade)
{
	if(Character && Character->GetGrenade())
	{
		Character->GetGrenade()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::UpdateHUDGernades()
{
	Controller = Controller == nullptr?Cast<AMyPlayerController>(Character->Controller):Controller;
	// if(Character && Character->HasAuthority())
	if(Controller)
	{
		Controller->SetHUDGrenades(Grenades);
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if(EquippedWeapon == nullptr)return;
	
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr?Cast<AMyPlayerController>(Character->Controller):Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip)
{
	if(Character == nullptr || WeaponToEquip == nullptr) return;
	
	if(WeaponToEquip->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation
		(this,
			WeaponToEquip->EquipSound,
			Character->GetActorLocation());
	}
}

void UCombatComponent::ServerTossGrenade_Implementation()
{
	if(Grenades <= 0) return;
	CombatState = ECombatState::ECS_TossGrenade;
	if(Character)
	{
		Character->PlayTossGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowGrenade(true);
	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateHUDGernades();
}


void UCombatComponent::OnRep_Aiming()
{
	if(Character && Character->IsLocallyControlled())
	{
		bAiming = bAimButtonPressed;
		EquippedWeapon->SwitchAim(bAiming);
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if(EquippedWeapon == nullptr) return;
	if(bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, ZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if(Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

bool UCombatComponent::CanFire()
{
	if(EquippedWeapon == nullptr)return false;
	if(EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun
		&& !EquippedWeapon->AmmoEqualsZero()
		&& CombatState == ECombatState::ECS_Reloading)
	{
		bLocallyReloading = false;
		return true;
	}
	if(bLocallyReloading) return false;
	return !EquippedWeapon->AmmoEqualsZero() && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr?Cast<AMyPlayerController>(Character->Controller):Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	if(CombatState == ECombatState::ECS_Reloading
		&& EquippedWeapon!=nullptr
		&& EquippedWeapon->GetWeaponType()==EWeaponType::EWT_ShotGun
		&& CarriedAmmo == 0)
	{
		JumpToShotGunEnd();
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRLAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_ShotGun, StartingShotGunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperRifleAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGLAmmo);
}

void UCombatComponent::OnRep_CombatState()
{
	switch(CombatState)
	{
	case ECombatState::ECS_Reloading:
		if(Character && !Character->IsLocallyControlled())
		{
			HandleReload();
		}
		break;
	case ECombatState::ECS_TossGrenade:
		if(Character && !Character->IsLocallyControlled())
		{
			Character->PlayTossGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
			ShowGrenade(true);
		}
		break;
	case ECombatState::ECS_SwapingWeapons:
		if(Character && !Character->IsLocallyControlled())
		{
			Character->PlaySwapMontage();
		}
		break;
	default:
		break;		
	}
}

void UCombatComponent::UpdateAmmoValue()
{
	if(EquippedWeapon==nullptr || Character ==nullptr)return;
	
	int32 ReloadAmount = AmountToReload();
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	EquippedWeapon->AddAmmo(ReloadAmount);
	Controller = Controller == nullptr?Cast<AMyPlayerController>(Character->Controller):Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::UpdateShotGunAmmoValue()
{
	if(EquippedWeapon==nullptr || Character ==nullptr)return;
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	EquippedWeapon->AddAmmo(1);
	Controller = 
		Controller ==nullptr?
			Cast<AMyPlayerController>(Character->Controller):Controller;
	
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	
	if(EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		JumpToShotGunEnd();
	}
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGernades();
}

void UCombatComponent::SetCrosshairShootingFactor(float f)
{
	if(CrosshairShootingFactor<2.f)
	{
		CrosshairShootingFactor += f;
	}
}

void UCombatComponent::FinishReloading()
{
	if(Character ==nullptr)return;
	bLocallyReloading = false;
	if(Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValue();
	}
}

void UCombatComponent::FinishSwap()
{
	if(Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

void UCombatComponent::FinishSwapAttachWeapons()
{
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	PlayEquipWeaponSound(EquippedWeapon);
	ZoomedFOV = EquippedWeapon->GetZoomedFOV();
	ZoomInterpSpeed = EquippedWeapon->GetZoomInterpSpeed();
	
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryWeapon);
}

void UCombatComponent::JumpToShotGunEnd()
{
	if(UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
			AnimInstance && Character->GetReloadMontage() &&
			EquippedWeapon
			&& EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun)
	{
		AnimInstance->Montage_JumpToSection(FName("ShotGunEnd"));
	}
}

void UCombatComponent::LaunchGrenade()
{
	ShowGrenade(false);
	if(Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if(Character && GrenadeClass && Character->GetGrenade())
	{
		const FVector StartingLocation = Character->GetGrenade()->GetComponentLocation();
		const FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		if(UWorld* World = GetWorld())
		{
			AProjectile* Grenade = World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams
			);
		}
	}
}

void UCombatComponent::TossGrenadeFinish()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

bool UCombatComponent::CouldSwapWeapons() const
{
	return EquippedWeapon!=nullptr
		&& SecondaryWeapon!=nullptr
		&& CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	// TODO: 这里是bug，服务器会开两次枪
	
	if(Character == nullptr || (Character->IsLocallyControlled() && !Character->HasAuthority()))
	{
		return;
	}
	LocalFire(TraceHitTarget);
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(EquippedWeapon && !EquippedWeapon->GetAutoFire())
	{
		CombatState = ECombatState::ECS_Cooling;
	}
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	EquippedWeapon->SwitchAim(bAiming);
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed:BaseWalkSpeed;
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(Character == nullptr || WeaponToEquip == nullptr)return;
	if(CombatState != ECombatState::ECS_Unoccupied) return;

	if(WeaponToEquip->GetWeaponType() == EWeaponType::EWT_Bomb)
	{
		bHoldingBomb = true;
		Character->Crouch();
		Character->GetCharacterMovement()->bOrientRotationToMovement = true;
		Character->bUseControllerRotationYaw = false;
		WeaponToEquip->SetWeaponState(EWeaponState::EWS_Equipped);
		WeaponToEquip->SetOwner(Character);
		AttachBombToLeftHand(WeaponToEquip);
		EquippedBomb = WeaponToEquip;
		Character->PlaySound(ECharacterSoundType::EST_BombSound);
	}
	else
	{
		if(EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
		{
			EquipSecondaryWeapon(WeaponToEquip);
		}
		else 
		{
			EquipPrimaryWeapon(WeaponToEquip);
		}
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		Character->PlaySound(ECharacterSoundType::EST_EquipSound);
	}
	
	
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	DropEquippedWeapon();
	
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();

	UpdateCarriedAmmo();
	
	PlayEquipWeaponSound(WeaponToEquip);
	ZoomedFOV = WeaponToEquip->GetZoomedFOV();
	ZoomInterpSpeed = WeaponToEquip->GetZoomInterpSpeed();
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(WeaponToEquip);
	SecondaryWeapon->SetOwner(Character);
	PlayEquipWeaponSound(WeaponToEquip);
}

void UCombatComponent::PlayNoAmmoSound()
{
	if(EquippedWeapon == nullptr ||
		EquippedWeapon->NoAmmoSound == nullptr ||
		Character == nullptr)
	{
		return;
	}
	UGameplayStatics::PlaySoundAtLocation(
		this,
		EquippedWeapon->NoAmmoSound,
		Character->GetActorLocation()
		);
	bPlayNoAmmoSound = false;
}

void UCombatComponent::Reload()
{
	if(CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied
		&& EquippedWeapon
		&& !EquippedWeapon->IsFull()
		&& !bLocallyReloading)
	{
		ServerReload();
		HandleReload();
		bLocallyReloading = true;
	}
}

void UCombatComponent::SwapWeapons()
{
	if(CombatState != ECombatState::ECS_Unoccupied || Character == nullptr)return;
	Character->PlaySwapMontage();
	CombatState = ECombatState::ECS_SwapingWeapons;
	
	AWeapon* LastEquippedWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = LastEquippedWeapon;
	
}

void UCombatComponent::ServerReload_Implementation()
{
	if(Character == nullptr)return;
	if(!Character->IsLocallyControlled())
	{
		HandleReload();
	}
	CombatState = ECombatState::ECS_Reloading;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
	DOREPLIFETIME(UCombatComponent, bHoldingBomb);
}

void UCombatComponent::OnRep_HoldingBomb()
{
	if(Character && Character->GetCharacterMovement() && Character->IsLocallyControlled())
	{
		if(bHoldingBomb)
		{
			Character->Crouch();
			Character->GetCharacterMovement()->bOrientRotationToMovement = true;
			Character->bUseControllerRotationYaw = false;
		}
		else
		{
			Character->UnCrouch();
			if(EquippedWeapon!=nullptr)
			{
				Character->GetCharacterMovement()->bOrientRotationToMovement = false;
				Character->bUseControllerRotationYaw = true;
			}
		}
	}
}