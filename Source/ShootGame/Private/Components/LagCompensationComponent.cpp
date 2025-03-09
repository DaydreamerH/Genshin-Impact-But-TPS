// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/LagCompensationComponent.h"

#include "Components/BoxComponent.h"
#include "Player/PlayerCharacter.h"

ULagCompensationComponent::ULagCompensationComponent()
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
	for(auto& BoxPair : Character->HitCollisionBoxes)
	{
		FBoxInformation BoxInformation;
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

void ULagCompensationComponent::ServerSideRewind(APlayerCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, float HitTime)
{
	if(HitCharacter == nullptr
		|| HitCharacter->GetLagCompensation()
		|| HitCharacter->GetLagCompensation()->FrameHistory.GetHead() == nullptr
		|| HitCharacter->GetLagCompensation()->FrameHistory.GetTail() == nullptr)
	{
		return;
	}
	
	const TDoubleLinkedList<FFramePackage>& History
		= HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	
	if(OldestHistoryTime>HitTime)
	{
		return;
	}
	FFramePackage FrameToCheck;
	bool bShouldInterpelate = true;
	if(OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpelate = false;
	}
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	
	if(NewestHistoryTime<=HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpelate = false;
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
		bShouldInterpelate = false;
	}
	if(bShouldInterpelate)
	{
		
	}
}


void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(FrameHistory.Num()<=1)
	{
		FFramePackage ThisFrame;
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

