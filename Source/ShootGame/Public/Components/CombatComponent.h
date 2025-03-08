
#pragma once

#include "CoreMinimal.h"
#include "CombatStates.h"
#include "Components/ActorComponent.h"
#include "Weapon/Projectile.h"
#include "Weapon/WeaponTypes.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 15000

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTGAME_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	friend class APlayerCharacter;

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void Reload();

	void SwapWeapons();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ShotGunShellReload();

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);
protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	
	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget );

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void LocalFire(const FVector_NetQuantize& TraceHitTarget);

	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotGun();
	
	UFUNCTION(Server, Reliable)
	void ServerCooldown();

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 AmountToReload();

	void TossGrenade();

	UFUNCTION(Server, Reliable)
	void ServerTossGrenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile>GrenadeClass;
	
	void DropEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToBackpack(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void ShowGrenade(bool bShowGrenade);

	void UpdateHUDGernades();

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);

	void PlayNoAmmoSound();
private:
	UPROPERTY()
	APlayerCharacter* Character;
	UPROPERTY()
	class AMyPlayerController* Controller;
	UPROPERTY()
	class APlayerHUD* HUD;

	float CrosshairVelocityFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	FVector HitTarget;

	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	float CurrentFOV;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	bool CanFire();

	UPROPERTY(ReplicatedUsing=OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;
	UPROPERTY(EditAnywhere)
	int32 StartingRLAmmo = 2;
	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 10;
	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 40;
	UPROPERTY(EditAnywhere)
	int32 StartingShotGunAmmo = 24;
	UPROPERTY(EditAnywhere)
	int32 StartingSniperRifleAmmo = 20;
	UPROPERTY(EditAnywhere)
	int32 StartingGLAmmo = 20;
	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 500;
	
	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombatState CombatState;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValue();
	void UpdateShotGunAmmoValue();

	bool bPlayNoAmmoSound = true;

	float ShotGunReloadTimeSpace = 0.1f;
	float ShotGunLastReloadTime = 0.f;

	UPROPERTY(ReplicatedUsing=OnRep_Grenades)
	int32 Grenades = 2;
	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;
	UFUNCTION()
	void OnRep_Grenades();
public:	
	
	void SetCrosshairShootingFactor(float f);

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	void JumpToShotGunEnd();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();
	
	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);
	
	UFUNCTION(BlueprintCallable)
	void TossGrenadeFinish();

	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	bool CouldSwapWeapons() const;
};
