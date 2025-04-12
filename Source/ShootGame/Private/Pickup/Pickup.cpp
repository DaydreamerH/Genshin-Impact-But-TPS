#include "Pickup/Pickup.h"

#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	OverlapShpere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapShpere->SetupAttachment(RootComponent);
	OverlapShpere->SetSphereRadius(100.f);
	OverlapShpere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapShpere->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapShpere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(OverlapShpere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	PickupEffectComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("PickupEffectComponent"));
	PickupEffectComponent->SetupAttachment(RootComponent);
	PickupEffectComponent->bAutoActivate = true;
}

void APickup::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			BindOverlapTimer,
			this,
			&ThisClass::BindOverlapTimerFinished,
			BindOverlapTime
		);
	}
}

void APickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Overlap logic here...
}

void APickup::BindOverlapTimerFinished()
{
	if (HasAuthority())
	{
		OverlapShpere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	}
}

void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PickupMesh)
	{
		PickupMesh->AddLocalRotation(FRotator(0.f, BaseTurnRate * DeltaTime, 0.f));
	}
}

void APickup::Destroyed()
{
	Super::Destroyed();

	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PickupSound,
			GetActorLocation()
		);
	}

	if (PickupEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			this,
			PickupEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}
}

