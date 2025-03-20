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

	UFUNCTION(Server, Reliable)
	void ServerProjectileScoreRequest(
		APlayerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
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
		float HitTime) const;

	FServerSideRewindResult ProjectileServerSideRewindResult(
		APlayerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	) const;


	static FFramePackage InterpBetweenFrames(
		const FFramePackage& OlderFrame,
		const FFramePackage& YoungerFrame,
		float HitTime);

	FServerSideRewindResult ConfirmHit(
		const FFramePackage& FramePackage,
		APlayerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation) const;

	static void CacheBoxPosition(APlayerCharacter* HitCharacter, FFramePackage& OutFramePackage);

	static void MoveBoxes(APlayerCharacter* HitCharacter, const FFramePackage& Package);

	static void ResetHitBoxes(APlayerCharacter* HitCharacter, const FFramePackage& Package);

	static void EnableCharacterMeshCollision(const APlayerCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	static FFramePackage GetFrameToCheck(APlayerCharacter* HitCharacter, float HitTime);

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
	) const;

	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& FramePackage,
		APlayerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& InitializeVelocity,
		float HitTime
	) const;
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

