// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/LagCompensationComponent.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerCharacter.h"
#include "ShootGame/ShootGame.h"
#include "Weapon/Weapon.h"

ULagCompensationComponent::ULagCompensationComponent(): Character(nullptr), Controller(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	// FFramePackage Package;
	// SaveFramePackage(Package);
	// ShowFramePackage(Package, FColor::Green);
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<APlayerCharacter>(GetOwner()) : Character;
	if(Character == nullptr)return;
	Package.Time = GetWorld()->GetTimeSeconds();
	Package.HitCharacter = Character;
	for(auto& BoxPair : Character->HitCollisionBoxes)
	{
		if(BoxPair.Value == nullptr)continue;
		FBoxInformation BoxInformation = FBoxInformation();
		BoxInformation.Location = BoxPair.Value->GetComponentLocation();
		BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
		BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
		Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
	}
}

void ULagCompensationComponent::ShowFramePackage(FFramePackage& Package, const FColor& Color) const
{
	for(auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			4.f
		);
	}
}

void ULagCompensationComponent::SaveFramePackage()
{
	if(Character == nullptr || !Character->HasAuthority())return;
	if(FrameHistory.Num()<=1)
	{
		FFramePackage ThisFrame = FFramePackage();
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		float HistoryLength =
			FrameHistory.GetHead()->GetValue().Time 
				- FrameHistory.GetTail()->GetValue().Time;
		while(HistoryLength>MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time
				- FrameHistory.GetTail()->GetValue().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
		// ShowFramePackage(ThisFrame, FColor::Cyan);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(APlayerCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
                                                                    const FVector_NetQuantize& HitLocation, float HitTime) const
{
	const FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewindResult(APlayerCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime) const
{
	const FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

FShotGunServerSideRewindResult ULagCompensationComponent::ShotGunServerSideRewind(
	const TArray<APlayerCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FramesToCheck = TArray<FFramePackage>();
	for(APlayerCharacter* HitCharacter : HitCharacters)
	{
		FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
		FrameToCheck.HitCharacter = FrameToCheck.HitCharacter == nullptr ? HitCharacter : FrameToCheck.HitCharacter;
		FramesToCheck.Add(FrameToCheck);
	}
	return ShotGunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(APlayerCharacter* HitCharacter, float HitTime)
{
	if(HitCharacter == nullptr
			|| HitCharacter->GetLagCompensation() == nullptr
			|| HitCharacter->GetLagCompensation()->FrameHistory.GetHead() == nullptr
			|| HitCharacter->GetLagCompensation()->FrameHistory.GetTail() == nullptr)
	{
		return FFramePackage();
	}
	
	const TDoubleLinkedList<FFramePackage>& History
		= HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	
	if(OldestHistoryTime>HitTime)
	{
		return FFramePackage();
	}
	FFramePackage FrameToCheck = FFramePackage();
	bool bShouldInterpolate = true;
	if(OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}

	if(const float NewestHistoryTime = History.GetHead()->GetValue().Time;
		NewestHistoryTime<=HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;
	while(Older->GetValue().Time>HitTime)
	{
		if(Older->GetNextNode() == nullptr)
		{
			break;
		}
		Older = Older->GetNextNode();
		if(Older->GetValue().Time>HitTime)
		{
			Younger = Older;
		}
	}
	if(Older->GetValue().Time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}
	if(bShouldInterpolate)
	{
		FrameToCheck = InterpBetweenFrames(
			Older->GetValue(),
			Younger->GetValue(),
			HitTime);
	}
	
	FrameToCheck.HitCharacter = HitCharacter;
	return FrameToCheck;
}


FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame,
                                                             const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);

	FFramePackage InterpFramePackage = FFramePackage();
	InterpFramePackage.Time = HitTime;

	for(auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;

		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		FBoxInformation InterpBoxInfo;
		InterpBoxInfo.Location = FMath::VInterpTo(
			OlderBox.Location,
			YoungerBox.Location,
			1.f,
			InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(
			OlderBox.Rotation,
			YoungerBox.Rotation,
			1.f,
			InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}
	
	return InterpFramePackage;
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& FramePackage,
	APlayerCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation) const
{
	if(HitCharacter == nullptr)return FServerSideRewindResult();

	FFramePackage CurrentFrame;
	CacheBoxPosition(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, FramePackage);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);


	if(const UWorld* World = GetWorld())
	{
		// 头部检测
		UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("Head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox
		);
		if(ConfirmHitResult.bBlockingHit)
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{true, true};
		}
		else
		{
			// 身体检测
			for(auto& HitBoxPair:HitCharacter->HitCollisionBoxes)
			{
				if(HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
				}
			}
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			if(ConfirmHitResult.bBlockingHit)
			{
				ResetHitBoxes(HitCharacter, CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{true, false};
			}
		}
	}

	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{false, false};
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& FramePackage, APlayerCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& InitializeVelocity, float HitTime) const
{
	if(HitCharacter == nullptr)return FServerSideRewindResult();

	FFramePackage CurrentFrame;
	CacheBoxPosition(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, FramePackage);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
	
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("Head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);

	FPredictProjectilePathParams Params;
	Params.bTraceWithCollision = true;
	Params.MaxSimTime = MaxRecordTime;
	Params.LaunchVelocity = InitializeVelocity;
	Params.StartLocation = TraceStart;
	Params.SimFrequency = 15.f;
	Params.ProjectileRadius = 5.f;
	Params.TraceChannel = ECC_HitBox;
	Params.ActorsToIgnore.Add(GetOwner());
	FPredictProjectilePathResult Result;
	UGameplayStatics::PredictProjectilePath(this, Params, Result);

	if(Result.HitResult.bBlockingHit)
	{
		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		return FServerSideRewindResult{true, true};
	}
	else
	{
		for(auto& HitBoxPair:HitCharacter->HitCollisionBoxes)
		{
			if(HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			}
		}
		UGameplayStatics::PredictProjectilePath(this, Params, Result);
		if(Result.HitResult.bBlockingHit)
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{true, false};
		}
	}

	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{false, false};
}

FShotGunServerSideRewindResult ULagCompensationComponent::ShotGunConfirmHit(const TArray<FFramePackage>& FramePackages,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations) const
{
	for(auto& Frame : FramePackages)
	{
		if(Frame.HitCharacter == nullptr)
		{
			return FShotGunServerSideRewindResult();
		}
	}
	FShotGunServerSideRewindResult ShotGunResult = FShotGunServerSideRewindResult();
	TArray<FFramePackage> CurrentFrames = TArray<FFramePackage>();

	for(auto& Frame : FramePackages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.HitCharacter = Frame.HitCharacter;
		CacheBoxPosition(Frame.HitCharacter, CurrentFrame);
		CurrentFrames.Add(CurrentFrame);
		MoveBoxes(Frame.HitCharacter, Frame);
		EnableCharacterMeshCollision(Frame.HitCharacter, ECollisionEnabled::NoCollision);
	}
	
	for(auto& Frame: FramePackages)
	{
		UBoxComponent* HeadBox = Frame.HitCharacter->HitCollisionBoxes[FName("Head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
	}
	const UWorld* World = GetWorld();
	if(World==nullptr)return FShotGunServerSideRewindResult();
	//头部检测
	for(auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult = FHitResult();
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if(World)
		{
			World->LineTraceSingleByChannel(
            		ConfirmHitResult,
            		TraceStart,
            		TraceEnd,
            		ECC_HitBox);
		}
		if(APlayerCharacter* PlayerCharacter
			= Cast<APlayerCharacter>(ConfirmHitResult.GetActor()))
		{
			if(ShotGunResult.HeadShots.Contains(PlayerCharacter))
			{
				ShotGunResult.HeadShots[PlayerCharacter]++;
			}
			else
			{
				ShotGunResult.HeadShots.Emplace(PlayerCharacter, 1);
			}
		}
	}

	// 头部关闭，身体打开
	for(auto& Frame:FramePackages)
	{
		for(auto& HitBoxPair:Frame.HitCharacter->HitCollisionBoxes)
		{
			if(HitBoxPair.Value!=nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			}
		}
		UBoxComponent* HeadBox = Frame.HitCharacter->HitCollisionBoxes[FName("Head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 身体检测
	for(auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult = FHitResult();
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if(World)
		{
			World->LineTraceSingleByChannel(
					ConfirmHitResult,
					TraceStart,
					TraceEnd,
					ECC_HitBox);
		}
		if(APlayerCharacter* PlayerCharacter
			= Cast<APlayerCharacter>(ConfirmHitResult.GetActor()))
		{
			if(ShotGunResult.BodyShots.Contains(PlayerCharacter))
			{
				ShotGunResult.BodyShots[PlayerCharacter]++;
			}
			else
			{
				ShotGunResult.BodyShots.Emplace(PlayerCharacter, 1);
			}
		}
	}
	for(auto& CurrentFrame:CurrentFrames)
	{
		ResetHitBoxes(CurrentFrame.HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(CurrentFrame.HitCharacter, ECollisionEnabled::QueryAndPhysics);
	}
	return ShotGunResult;
}

void ULagCompensationComponent::CacheBoxPosition(APlayerCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if(HitCharacter == nullptr)return;
	for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if(HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}


void ULagCompensationComponent::MoveBoxes(APlayerCharacter* HitCharacter, const FFramePackage& Package)
{
	if(HitCharacter == nullptr)return;
	for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if(HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(APlayerCharacter* HitCharacter, const FFramePackage& Package)
{
	if(HitCharacter == nullptr)return;
	for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if(HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(const APlayerCharacter* HitCharacter,
                                                             ECollisionEnabled::Type CollisionEnabled)
{
	if(HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	SaveFramePackage();
	
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(APlayerCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if(HitCharacter && DamageCauser && Character && Confirm.bHitConfirmed)
	{
		const float DamageToCause = Confirm.bHeadShot ?
			DamageCauser->GetHeadShotDamage() : DamageCauser->GetDamage();
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			DamageToCause,
			Character->Controller,
			DamageCauser,
			UDamageType::StaticClass()
		);
	}
}

void ULagCompensationComponent::ServerShotGunScoreRequest_Implementation(const TArray<APlayerCharacter*>& HitCharacters,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime,
	AWeapon* DamageCauser)
{
	FShotGunServerSideRewindResult Confirm
		= ShotGunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);
	
	for(auto& HitCharacter : HitCharacters)
	{
		if(HitCharacter == nullptr)continue;
		float TotalDamage = 0.f;
		if(Confirm.HeadShots.Contains(HitCharacter))
		{
			TotalDamage += Confirm.HeadShots[HitCharacter]*DamageCauser->GetHeadShotDamage();
		}
		if(Confirm.BodyShots.Contains(HitCharacter))
		{
			TotalDamage += Confirm.BodyShots[HitCharacter]*DamageCauser->GetDamage();	
		}
		
		if(HitCharacter && Character && Character->Controller)
		{
			UGameplayStatics::ApplyDamage(
				HitCharacter,
				TotalDamage,
				Character->Controller,
				DamageCauser,
				UDamageType::StaticClass()
			);
		}
	}
}

void ULagCompensationComponent::ServerProjectileScoreRequest_Implementation(APlayerCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	if(const FServerSideRewindResult Confirm
		= ProjectileServerSideRewindResult(HitCharacter, TraceStart, InitialVelocity, HitTime);
		HitCharacter && Character && Confirm.bHitConfirmed)
	{
		const float DamageToCause = Confirm.bHeadShot ?
			Character->GetEquippedWeapon()->GetHeadShotDamage()
				: Character->GetEquippedWeapon()->GetDamage();
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			DamageToCause,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}