// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TurningPlace.h"
#include "GameFramework/Character.h"
#include "../Plugins/EnhancedInput/Source/EnhancedInput/Public/inputActionValue.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class SHOOTGAME_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 玩家索引，服务器与客户端同步
	UPROPERTY(ReplicatedUsing=OnRep_PlayerIndex)
	int32 PlayerIndex = -1;

	// Replicated回调函数
	UFUNCTION()
	void OnRep_PlayerIndex();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	void Elim(); // 只在服务器上

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	FTimerHandle ElimTimer;
	FTimerHandle HealthRecoveryTimerHandle;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 2.5f;
	
	void ElimTimerFinished();
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnActionMoveForward(const FInputActionValue& InputActionValue);
	UFUNCTION()
	void OnActionMoveRight(const FInputActionValue& InputActionValue);
	UFUNCTION()
	void OnActionLookUp(const FInputActionValue& InputActionValue);
	UFUNCTION()
	void OnActionLookRight(const FInputActionValue& InputActionValue);
	UFUNCTION()
	void OnActionJump(const FInputActionValue& InputActionValue);
	UFUNCTION()
	void OnActionEquip(const FInputActionValue& InputActionValue);
	UFUNCTION()
	void OnActionCrouch(const FInputActionValue& InputActionValue);
	UFUNCTION()
	void OnActionAimPressed(const FInputActionValue& InputActionValue);
	UFUNCTION()
	void OnActionAimReleased(const FInputActionValue& InputActionValue);
	UFUNCTION()
	void OnActionFirePressed(const FInputActionValue& InputActionValue);
	UFUNCTION()
	void OnActionFireReleased(const FInputActionValue& InputActionValue);

	void AimOffset(float DeltaTime);
	
	void UpdateMPC();

	virtual void Jump() override;

	UFUNCTION()
	void ReceiveDamage
	(AActor* DamageActor, float Damage,
		const UDamageType* DamageType,
		class AController* InstigatorController,
		AActor* DamageCauser);

	
private:
	
	// 相机模块
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	// 输入事件
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput|Action", meta = (AllowPrivateAccess = "true"))
	class UInputAction* IA_MoveForward;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput|Action", meta = (AllowPrivateAccess = "true"))
	class UInputAction* IA_MoveRight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput|Action", meta = (AllowPrivateAccess = "true"))
	class UInputAction* IA_LookUp;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput|Action", meta = (AllowPrivateAccess = "true"))
	class UInputAction* IA_LookRight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput|Action", meta = (AllowPrivateAccess = "true"))
	class UInputAction* IA_Jump;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput|Action", meta = (AllowPrivateAccess = "true"))
	class UInputAction* IA_Equip;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput|Action", meta = (AllowPrivateAccess = "true"))
	class UInputAction* IA_Crouch;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput|Action", meta = (AllowPrivateAccess = "true"))
	class UInputAction* IA_Aim;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput|Action", meta = (AllowPrivateAccess = "true"))
	class UInputAction* IA_Fire;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput|Action", meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* InputMappingContext;

	// 获取MPC
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UMaterialParameterCollection* MPC_Position;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon) const;

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	UFUNCTION(Server, Reliable)
	void ServerOnActionEquip();

	float AO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	float InterpAO_Yaw;

	UPROPERTY(EditAnywhere, Category=Combat)
	class UAnimMontage* FireWeaponMontage;
	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* HitReactMontage;
	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* ElimMontage;

	float CurrentRadius;

	void HideCamera();
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	UPROPERTY(EditAnywhere, Category= "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing=OnRep_Health, VisibleAnywhere, Category="Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	class AMyPlayerController* PlayerController;

	void UpdateHUDHealth();

	bool bElimmed = false;

	void StartHealthRecovery();

	UFUNCTION()
	void RecoverHealthTick();
	
public:	
	void SetOverlappingWeapon(AWeapon* Weapon);

	bool IsWeaponEquipped() const;

	bool IsAiming() const;

	FORCEINLINE float GetAO_Yaw() const {return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const {return AO_Pitch; }
	AWeapon* GetEuippedWeapon() const;

	FORCEINLINE ETurningInPlace GetTurningInPlace() const {return TurningInPlace; }

	void PlayFireMontage(bool bAiming) const;
	void PlayHitReactMontage() const;
	void PlayElimMontage() const;
	
	FVector GetHitTarget() const;

	FORCEINLINE UCameraComponent* GetFollowCamera() const {return FollowCamera;}

	void SetCrosshairShootingFactor() const;

	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
};
