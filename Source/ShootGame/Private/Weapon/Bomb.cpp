// Bomb.cpp

#include "Weapon/Bomb.h"
#include "Components/SphereComponent.h"
#include "ShootGame/ShootGame.h"

ABomb::ABomb()
{
	BombMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BombMesh"));
	BombMesh->SetCollisionObjectType(ECC_Bomb);
	BombMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BombMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	BombMesh->SetCollisionResponseToChannel(ECC_BombStorage, ECR_Block);
	BombMesh->SetGenerateOverlapEvents(true);
	
	RootComponent = BombMesh;

	GetWeaponMesh()->SetupAttachment(RootComponent);
}

void ABomb::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		BombMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void ABomb::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	BombMesh->DetachFromComponent(DetachmentTransformRules);
	OwnerPlayerCharacter = nullptr;
	OwnerPlayerController = nullptr;
	SetOwner(nullptr);
}

void ABomb::OnEquipped()
{
	ShowPickupWidget(false);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABomb::OnDropped()
{
	if (HasAuthority())
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}