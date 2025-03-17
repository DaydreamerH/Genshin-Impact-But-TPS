

#include "ShootGame/Public/Weapon/Weapon.h"

#include "AudioDevice.h"
#include "Components/CombatComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerCharacter.h"
#include "PlayerController/MyPlayerController.h"
#include "Weapon/BulletShell.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	SetRootComponent(WeaponMesh);
	
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);

	SphereRadius = NormalSphereRadius;
}

void AWeapon::SwitchAim(bool bAiming)
{
	if(bAiming)
	{
		switch (WeaponType)
		{
		case EWeaponType::EWT_SniperRifle:
			bUseScatter = false;
			break;
		case EWeaponType::EWT_Pistol:
			SphereRadius = AimingSphereRadius;
			break;
		case EWeaponType::EWT_ShotGun:
			SphereRadius = AimingSphereRadius;
			break;
		default:
			break;
		}
	}
	else
	{
		switch (WeaponType)
		{
		case EWeaponType::EWT_SniperRifle:
			bUseScatter = true;
			break;
		case EWeaponType::EWT_Pistol:
			SphereRadius = NormalSphereRadius;
			break;
		case EWeaponType::EWT_ShotGun:
			SphereRadius = NormalSphereRadius;
			break;
		default:
			break;
		}
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if(PickupWidget)PickupWidget->SetVisibility(false);
	
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);

	WeaponMesh->SetOverlayMaterial(OverlayMaterial);
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResule)
{
	APlayerCharacter* playerCharacter = Cast<APlayerCharacter>(OtherActor);
	if(playerCharacter)
	{
		playerCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APlayerCharacter* playerCharacter = Cast<APlayerCharacter>(OtherActor);
	if(playerCharacter)
	{
		playerCharacter->SetOverlappingWeapon(nullptr);
	}
}

/* void AWeapon::OnRep_Ammo()
{
	OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ? Cast<APlayerCharacter>(GetOwner()):OwnerPlayerCharacter;
	if(WeaponType == EWeaponType::EWT_ShotGun && OwnerPlayerCharacter && OwnerPlayerCharacter->GetCombat() && IsFull())
	{
		OwnerPlayerCharacter->GetCombat()->JumpToShotGunEnd();
	}
	SetHUDAmmo();
}*/

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if(HasAuthority())
	{
		return;
	}
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if(HasAuthority())return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapcitiy);
	OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr ?
		Cast<APlayerCharacter>(GetOwner()):OwnerPlayerCharacter;

	if(OwnerPlayerCharacter && OwnerPlayerCharacter->GetCombat() && IsFull())
	{
		OwnerPlayerCharacter->GetCombat()->JumpToShotGunEnd();
	}
	SetHUDAmmo();
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapcitiy);
	SetHUDAmmo();
	if(HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else if(OwnerPlayerCharacter&&OwnerPlayerCharacter->IsLocallyControlled())
	{
		++Sequence;
	}
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapcitiy);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
}

void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();

	OnWeaponStateChanged.Broadcast(WeaponState);
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippeedSecondary();
		break;
	default:
		break;
	}
}

void AWeapon::OnEquipped()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetOverlayMaterial(nullptr);

	OwnerPlayerCharacter = OwnerPlayerCharacter==nullptr?
		Cast<APlayerCharacter>(GetOwner()):OwnerPlayerCharacter;
	if(OwnerPlayerCharacter && bUseServerSideRewind)
	{
		OwnerPlayerController =
			OwnerPlayerController==nullptr?
				Cast<AMyPlayerController>(OwnerPlayerCharacter->GetController()):OwnerPlayerController;
		if(OwnerPlayerController && HasAuthority() && !OwnerPlayerController->HighPingDelegate.IsBound())
		{
			OwnerPlayerController->HighPingDelegate.AddDynamic(this, &ThisClass::OnPingTooHigh);
		}
	}
}

void AWeapon::OnDropped()
{
	if(HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetOverlayMaterial(OverlayMaterial);

	OwnerPlayerCharacter = OwnerPlayerCharacter==nullptr?
		Cast<APlayerCharacter>(GetOwner()):OwnerPlayerCharacter;
	if(OwnerPlayerCharacter && bUseServerSideRewind)
	{
		OwnerPlayerController =
			OwnerPlayerController==nullptr?
				Cast<AMyPlayerController>(OwnerPlayerCharacter->GetController()):OwnerPlayerController;
		if(OwnerPlayerController && HasAuthority() && OwnerPlayerController->HighPingDelegate.IsBound())
		{
			OwnerPlayerController->HighPingDelegate.RemoveDynamic(this, &ThisClass::OnPingTooHigh);
		}
	}
}

void AWeapon::OnEquippeedSecondary()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetOverlayMaterial(nullptr);

	OwnerPlayerCharacter = OwnerPlayerCharacter==nullptr?
		Cast<APlayerCharacter>(GetOwner()):OwnerPlayerCharacter;
	if(OwnerPlayerCharacter && bUseServerSideRewind)
	{
		OwnerPlayerController =
			OwnerPlayerController==nullptr?
				Cast<AMyPlayerController>(OwnerPlayerCharacter->GetController()):OwnerPlayerController;
		if(OwnerPlayerController && HasAuthority() && OwnerPlayerController->HighPingDelegate.IsBound())
		{
			OwnerPlayerController->HighPingDelegate.RemoveDynamic(this, &ThisClass::OnPingTooHigh);
		}
	}
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	// DOREPLIFETIME(AWeapon, Ammo);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
}

void AWeapon::Fire(const FVector& HitTarget)
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
	}
	
}

void AWeapon::Dropped()
{
	if(WeaponType == EWeaponType::EWT_Pistol || WeaponType == EWeaponType::EWT_AssultRifle)
	{
		Destroy();
	}
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachmentTransformRules);
	OwnerPlayerCharacter=nullptr;
	OwnerPlayerController=nullptr;
	SetOwner(nullptr);
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if(Owner == nullptr)
	{
		OwnerPlayerCharacter = nullptr;
		OwnerPlayerController = nullptr;
	}
	else
	{
		OwnerPlayerCharacter = OwnerPlayerCharacter == nullptr
			? Cast<APlayerCharacter>(Owner) :OwnerPlayerCharacter;
		if (OwnerPlayerCharacter
			&& OwnerPlayerCharacter->GetEquippedWeapon()
			&& OwnerPlayerCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
	}
}

void AWeapon::SetHUDAmmo()
{
	OwnerPlayerCharacter = OwnerPlayerCharacter==nullptr?
		Cast<APlayerCharacter>(GetOwner()):OwnerPlayerCharacter;
	if(OwnerPlayerCharacter)
	{
		OwnerPlayerController =
			OwnerPlayerController==nullptr?
				Cast<AMyPlayerController>(OwnerPlayerCharacter->GetController()):OwnerPlayerController;

		if(OwnerPlayerController)
		{
			OwnerPlayerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}


FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget) const
{
	const USkeletalMeshSocket*
			MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket == nullptr)
	{
		return FVector();
	}
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform((GetWeaponMesh()));
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized*DistanceToSphere;

	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;

	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH);
}

