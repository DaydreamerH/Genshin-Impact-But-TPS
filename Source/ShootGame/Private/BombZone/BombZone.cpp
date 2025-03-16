

#include "BombZone/BombZone.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "GameState/ShootGameState.h"
#include "PlayerController/MyPlayerController.h"
#include "ShootGame/ShootGame.h"
#include "Weapon/Bomb.h"

ABombZone::ABombZone()
{
	PrimaryActorTick.bCanEverTick = false;
	ZoneSphere = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneSphere"));
	ZoneSphere->SetCollisionObjectType(ECC_BombStorage);
	ZoneSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ZoneSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	ZoneSphere->SetCollisionResponseToChannel(ECC_Bomb, ECR_Overlap);
	ZoneSphere->SetGenerateOverlapEvents(true);
}

void ABombZone::BeginPlay()
{
	Super::BeginPlay();
	if(HasAuthority())
	{
		ZoneSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &ABombZone::OnOverlapBegin);
		ZoneSphere->OnComponentEndOverlap.AddDynamic(this, &ABombZone::OnOverlapEnd);
	}
}

void ABombZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(ABomb* Bomb = Cast<ABomb>(OtherActor))
	{
		if(AShootGameState* GameState = GetWorld()->GetGameState<AShootGameState>())
		{
			if(Team == ETeam::ET_RedTeam)
			{
				GameState->RedTeamScores(Score);
			}
			else
			{
				GameState->BlueTeamScores(Score);
			}
		}
	}
}

void ABombZone::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if(ABomb* Bomb = Cast<ABomb>(OtherActor))
	{
		if(AShootGameState* GameState = GetWorld()->GetGameState<AShootGameState>())
		{
			if(Team == ETeam::ET_RedTeam)
			{
				GameState->RedTeamScores(-Score);
			}
			else
			{
				GameState->BlueTeamScores(-Score);
			}
		}
	}
}

