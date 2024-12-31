// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TurningPlace.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTGAME_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess))
	class APlayerCharacter* PlayerCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Character)
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = Character)
	bool bIsFalling;

	UPROPERTY(BlueprintReadOnly, Category = Character)
	bool bIsAccelerating;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsMoving;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bWeaponEquipped;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bElimmed = false;

	UPROPERTY()
	class AWeapon* EquippedWeapon;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsCrouched;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bAiming;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float YawOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float Lean;


	FRotator CharacterRotationLastFrame;

	FRotator CharacterRotation;

	FRotator DeltaRotation;

	UPROPERTY(BlueprintReadOnly, Category=Movement)
	float AO_Yaw;
	UPROPERTY(BlueprintReadOnly, Category=Movement)
	float AO_Pitch;
	UPROPERTY(BlueprintReadOnly, Category=Movement)
	FTransform LeftHandTransform;

	UPROPERTY(BlueprintReadOnly, Category=Movement)
	ETurningInPlace TurningInPlace;

	UPROPERTY(BlueprintReadOnly, Category=Movement)
	bool bUseFABRIK;
private:
	
};
