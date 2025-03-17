// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerSoundType.h"
#include "team.h"
#include "TurningPlace.h"
#include "GameFramework/Character.h"
#include "../Plugins/EnhancedInput/Source/EnhancedInput/Public/inputActionValue.h"
#include "Components/CombatComponent.h"
#include "Components/CombatStates.h"
#include "GameMode/ShootGameMode.h"
#include "PlayerCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

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

	void Elim(bool bPlayerLeftGame); // 只在服务器上

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);

	FTimerHandle ElimTimer;
	FTimerHandle HealthRecoveryTimerHandle;
	FTimerHandle ShieldRecoveryTimerHandle;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 2.5f;
	
	void ElimTimerFinished();

	UPROPERTY()
	TMap<FName, class UBoxComponent*> HitCollisionBoxes;

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	FOnLeftGame OnLeftGame;

	void SetTeamColor(ETeam Team);
	
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
	UFUNCTION()
	void OnActionReload(const FInputActionValue& InputActionValue);
	UFUNCTION()
	void OnActionTossGrenade(const FInputActionValue& InputActionValue);
	UFUNCTION()
	void OnActionSwapWeapons(const FInputActionValue& InputActionValue);
	UFUNCTION()
	void OnActionQuit(const FInputActionValue& InputActionValue);
	
	void AimOffset(float DeltaTime);
	
	void UpdateMPC();

	virtual void Jump() override;

	UFUNCTION()
	void ReceiveDamage
	(AActor* DamageActor, float Damage,
		const UDamageType* DamageType,
		class AController* InstigatorController,
		AActor* DamageCauser);

	void PollInit();

	void SetSpawnPoint();
	void OnPlayerStateInitialized();

	// 服务器倒带延迟补偿用
	UPROPERTY(EditAnywhere)
	UBoxComponent* Head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Neck;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Spine_1;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* RightShoulder;

	UPROPERTY(EditAnywhere)
	UBoxComponent* LeftShoulder;

	UPROPERTY(EditAnywhere)
	UBoxComponent* RightArmUpper;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* LeftArmUpper;

	UPROPERTY(EditAnywhere)
	UBoxComponent* RightArmMid;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* LeftArmMid;

	UPROPERTY(EditAnywhere)
	UBoxComponent* RightArmLower;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* LeftArmLower;

	UPROPERTY(EditAnywhere)
	UBoxComponent* RightHand;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* LeftHand;

	UPROPERTY(EditAnywhere)
	UBoxComponent* RightAss;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* LeftAss;

	UPROPERTY(EditAnywhere)
	UBoxComponent* RightLegUpper;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* LeftLegUpper;

	UPROPERTY(EditAnywhere)
	UBoxComponent* RightLegLower;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* LeftLegLower;

	UPROPERTY(EditAnywhere)
	UBoxComponent* RightFoot;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* LeftFoot;
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
	class UInputAction* IA_Reload;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput|Action", meta = (AllowPrivateAccess = "true"))
	class UInputAction* IA_TossGrenade;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput|Action", meta = (AllowPrivateAccess = "true"))
	class UInputAction* IA_SwapWeapons;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput|Action", meta = (AllowPrivateAccess = "true"))
	class UInputAction* IA_Quit;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;
	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;
	UPROPERTY(VisibleAnywhere)
	class ULagCompensationComponent* LagCompensation;

	UFUNCTION(Server, Reliable)
	void ServerOnActionEquip();

	UFUNCTION(Server, Reliable)
	void ServerOnActionSwapWeapons();

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
	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* ReloadMontage;
	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* TossGrenadeMontage;
	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* SwapMontage;
	
	float CurrentRadius;

	void HideCamera();
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	UPROPERTY(EditAnywhere, Category= "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing=OnRep_Health, VisibleAnywhere, Category="Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	UPROPERTY(EditAnywhere, Category= "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing=OnRep_Shield, EditAnywhere, Category="Player Stats")
	float Shield = 100.f;

	UPROPERTY(EditAnywhere, Category = "PlayerStats")
	float ShieldDropEverySecond = 5.f;
	void DropShield(float DeltaTime);

	UFUNCTION()
	void OnRep_Shield(float LastShield);
	
	UPROPERTY()
	class AMyPlayerController* PlayerController;

	bool bElimmed = false;
	bool bLeftGame = false;

	void StartHealthRecovery();
	UFUNCTION()
	void RecoverHealthTick();
	void StartShieldRecovery();
	UFUNCTION()
	void RecoverShieldTick();
	
	UPROPERTY()
	class AMyPlayerState* MyPlayerState;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachGrenade;

	UPROPERTY(EditAnywhere)
	USoundCue* HealSound;
	UPROPERTY(EditAnywhere)
	USoundCue* ShieldSound;
	UPROPERTY(EditAnywhere)
	USoundCue* JumpSound;
	UPROPERTY(EditAnywhere)
	USoundCue* SpeedSound;
	UPROPERTY(EditAnywhere)
	USoundCue* EquipSound;
	UPROPERTY(EditAnywhere)
	USoundCue* BombSound;
	UPROPERTY(EditAnywhere)
	USoundCue* KillSound;
	FTimerHandle SoundTimer;
	float SoundCoolDown = 10.f;
	bool bRecentlySound = false;
	UFUNCTION()
	void ResetRecentlySound();
	
	UPROPERTY(EditAnywhere)
	UMaterialInstance* FriendMat;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* EnemyMat;

	UPROPERTY()
	AShootGameMode* ShootGameMode;
	
public:
	void UpdateHUDHealth();
	void UpdateHUDShield();
	
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
	void PlayReloadMontage() const;
	void PlayTossGrenadeMontage() const;
	void PlaySwapMontage() const;
	
	FVector GetHitTarget() const;

	FORCEINLINE UCameraComponent* GetFollowCamera() const {return FollowCamera;}

	void SetCrosshairShootingFactor() const;

	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE void SetHealth(float Amount){ Health = Amount; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	FORCEINLINE float GetShield() const {return Shield;}
	FORCEINLINE float GetMaxShield() const {return MaxShield;}
	ECombatState GetCombatState() const;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	void CancelCombatComponentFireButtonPressed() const;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);
	FORCEINLINE UCombatComponent* GetCombat() const {return Combat;}
	FORCEINLINE UAnimMontage* GetReloadMontage() const {return ReloadMontage;}
	FORCEINLINE UStaticMeshComponent* GetGrenade() const {return AttachGrenade;}
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }
	
	bool IsLocallyReloading();

	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }

	FORCEINLINE AWeapon* GetEquippedWeapon() const { return Combat->EquippedWeapon; }

	FORCEINLINE bool isHoldingBomb() const {return Combat->bHoldingBomb;}

	void PlaySound(ECharacterSoundType SoundType);

	UFUNCTION(Client, Unreliable)
	void ClientPlaySound(ECharacterSoundType SoundType);

	void HandlePlaySound(ECharacterSoundType SoundType);
	
};
