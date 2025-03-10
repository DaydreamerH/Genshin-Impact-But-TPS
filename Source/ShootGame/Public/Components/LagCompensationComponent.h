// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Player/PlayerCharacter.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation>HitBoxInfo;

	UPROPERTY()
	APlayerCharacter* HitCharacter;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotGunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<APlayerCharacter*, uint32>HeadShots;
	UPROPERTY()
	TMap<APlayerCharacter*, uint32>BodyShots;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTGAME_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class APlayerCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		APlayerCharacter* PlayerCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime,
		AWeapon* DamageCauser
	);

	UFUNCTION(Server, Reliable)
	void ServerShotGunScoreRequest(
		const TArray<APlayerCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime,
		AWeapon* DamageCauser
	);
protected:
	virtual void BeginPlay() override;
	
	void SaveFramePackage(FFramePackage& Package);
	void SaveFramePackage();
	void ShowFramePackage(FFramePackage& Package, const FColor& Color) const;
	
	
	FServerSideRewindResult ServerSideRewind(
		APlayerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime);

	FFramePackage InterpBetweenFrames(
		const FFramePackage& OlderFrame,
		const FFramePackage& YoungerFrame,
		float HitTime);

	FServerSideRewindResult ConfirmHit(
		const FFramePackage& FramePackage,
		APlayerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation);

	void CacheBoxPosition(APlayerCharacter* HitCharacter, FFramePackage& OutFramePackage);

	void MoveBoxes(APlayerCharacter* HitCharacter, const FFramePackage& Package);

	void ResetHitBoxes(APlayerCharacter* HitCharacter, const FFramePackage& Package);

	void EnableCharacterMeshCollision(APlayerCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	FFramePackage GetFrameToCheck(APlayerCharacter* HitCharacter, float HitTime);

	FShotGunServerSideRewindResult ShotGunServerSideRewind(
		const TArray<APlayerCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
	);
	FShotGunServerSideRewindResult ShotGunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations
	);
	
private:
	UPROPERTY()
	APlayerCharacter* Character;

	UPROPERTY()
	class AMyPlayerController* Controller;
	
	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;

	
public:	
	
	
};

