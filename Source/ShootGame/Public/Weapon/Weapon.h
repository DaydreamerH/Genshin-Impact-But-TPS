// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "InitialState"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_MAX UMETA(DisplayName = "DefaultMax")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponStateChanged, EWeaponState, WeaponState);


UENUM(BlueprintType)
enum class EFireType: uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_ShotGun UMETA(DisplayName = "Shot Gun Weapon"),
	
	EFT_MAX UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class SHOOTGAME_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Fire(const FVector& HitTarget);
	virtual void Dropped();
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();
	void AddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;

	UPROPERTY(EditAnywhere)
	USoundCue* NoAmmoSound;

	UPROPERTY(EditAnywhere)
	EFireType FireType;

	UPROPERTY(EditAnywhere, Category="Weapon Scatter")
	bool bUseScatter = false;

	UFUNCTION()
	FVector TraceEndWithScatter(const FVector& HitTarget) const;

	FOnWeaponStateChanged OnWeaponStateChanged;

	void SwitchAim(bool bAiming);
protected:
	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnDropped();
	virtual void OnEquippedSecondary();
	
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);
	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	class UAnimationAsset* FireAnimation;
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ABulletShell> BulletShellClass;

	UPROPERTY(EditAnywhere)
	int32 Ammo;

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);
	// UFUNCTION()
	// void OnRep_Ammo();

	// 未被处理的服务器请求
	int32 Sequence = 0;
	
	void SpendRound();
	
	UPROPERTY(EditAnywhere)
	int32 MagCapacity = 30;

	UPROPERTY(EditAnywhere)
	bool AutoFire = true;

	UPROPERTY(EditAnywhere, Category="Weapon Scatter")
	float DistanceToSphere = 800.f;
	float SphereRadius = 75.f;
	UPROPERTY(EditAnywhere, Category="Weapon Scatter")
	float NormalSphereRadius = 75.f;
	UPROPERTY(EditAnywhere, Category="Weapon Scatter")
	float AimingSphereRadius = 50.f;
	
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerSideRewind = false;
	
	UPROPERTY()
	class APlayerCharacter* OwnerPlayerCharacter;
	UPROPERTY()
	class AMyPlayerController* OwnerPlayerController;

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

	void ApplyRecoil();
private:

	UPROPERTY(VisibleAnywhere, Category="WeaponProperties")
	class USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = "OnRep_WeaponState", Category="WeaponProperties")
	EWeaponState WeaponState = EWeaponState::EWS_Initial;

	UFUNCTION()
	void OnRep_WeaponState();
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties", meta=(AllowPrivateAccess="true"))
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere)
	float DeltaCrosshairShootingFactor = .2f;
	

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* OverlayMaterial;
	
	UPROPERTY(EditAnywhere, Category = "Recoil")
	float BaseVerticalRecoil = 2.0f;
	UPROPERTY(EditAnywhere, Category="Recoil")
	float VerticalRecoilIncrement = 2.f;
	UPROPERTY(EditAnywhere, Category = "Recoil")
	float HorizontalRecoil = 1.0f;
	UPROPERTY(EditAnywhere, Category="Recoil")
	float RecoilRecoverSpeed = 5.f;
	int32 ConsecutiveShotsCount = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	float RecoilResetTime = 1.f;
	float LastFireTime;

	FTimerHandle DestroyTimerHandle; 
	void OnDestroyTimerExpired();
	
public:
	void SetWeaponState(EWeaponState State);
	
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const{ return WeaponMesh; }

	UPROPERTY(EditAnywhere, Category=Crosshairs)
	class UTexture2D* CrosshairsCenter;
	UPROPERTY(EditAnywhere, Category=Crosshairs)
	UTexture2D* CrosshairsLeft;
	UPROPERTY(EditAnywhere, Category=Crosshairs)
	UTexture2D* CrosshairsTop;
	UPROPERTY(EditAnywhere, Category=Crosshairs)
	UTexture2D* CrosshairsRight;
	UPROPERTY(EditAnywhere, Category=Crosshairs)
	UTexture2D* CrosshairsBottom;

	FORCEINLINE float GetZoomedFOV() const {return ZoomedFOV;}
	FORCEINLINE float GetZoomInterpSpeed() const {return ZoomInterpSpeed;}
	FORCEINLINE float GetDeltaCrosshairShootingFactor() const {return DeltaCrosshairShootingFactor;}
	FORCEINLINE bool AmmoEqualsZero() const { return Ammo <= 0; }
	FORCEINLINE EWeaponType GetWeaponType() const {return WeaponType;}

	FORCEINLINE int32 GetAmmo() const {return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const {return MagCapacity;}

	FORCEINLINE bool GetAutoFire() const {return AutoFire;}

	FORCEINLINE bool IsFull() const { return Ammo >= MagCapacity; }

	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }
};
