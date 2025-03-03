
#pragma once

#include "CoreMinimal.h"
#include "CombatStates.h"
#include "Components/ActorComponent.h"
#include "Weapon/WeaponTypes.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 8000

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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void FireButtonPressed(bool bPressed);
protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	
	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget );

	UFUNCTION(NetMulticast, Reliable)
	void MuliticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerCooldown();

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 AmountToReload();
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
	
	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombatState CombatState;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValue();

	bool bPlayNoAmmoSound = true;
public:	
	
	void SetCrosshairShootingFactor(float f);

	UFUNCTION(BlueprintCallable)
	void FinishReloading();
};
