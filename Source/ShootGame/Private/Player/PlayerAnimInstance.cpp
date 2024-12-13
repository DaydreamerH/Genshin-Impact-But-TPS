// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootGame/Public/Player/PlayerAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ShootGame/Public/Player/PlayerCharacter.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
	
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(PlayerCharacter == nullptr)return ;

	FVector velocity = PlayerCharacter->GetVelocity();
	velocity.Z = 0.f;
	Speed = velocity.Size();

	bIsFalling = PlayerCharacter->GetCharacterMovement()->IsFalling();
	
	bIsMoving = (Speed > 0.0f || bIsFalling);
	
	bIsAccelerating =
		PlayerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size()>0? true : false;

	bWeaponEquipped = PlayerCharacter->IsWeaponEquipped();
	bIsCrouched = PlayerCharacter->bIsCrouched;
	bAiming = PlayerCharacter->IsAiming();

	FRotator AimRotation = PlayerCharacter->GetBaseAimRotation();
	FRotator MoveMentRotation = UKismetMathLibrary::MakeRotFromX(PlayerCharacter->GetVelocity());
	FRotator DeltaRote = UKismetMathLibrary::NormalizedDeltaRotator(MoveMentRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRote, DeltaSeconds, 15.f);
	YawOffset= DeltaRotation.Yaw;
	
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = PlayerCharacter->GetActorRotation();

	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw/DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);
}
