// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootGame/Public/Player/PlayerAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ShootGame/Public/Player/PlayerCharacter.h"
#include "Weapon/Weapon.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
	bIsMoving = false;
	bWeaponEquipped = false;
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(PlayerCharacter == nullptr)return ;

	FVector Velocity = PlayerCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsFalling = PlayerCharacter->GetCharacterMovement()->IsFalling();
	
	bIsMoving = (Speed > 0.0f || bIsFalling);
	
	bIsAccelerating =
		PlayerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size()>0? true : false;

	bWeaponEquipped = PlayerCharacter->IsWeaponEquipped();
	bIsCrouched = PlayerCharacter->bIsCrouched;
	bAiming = PlayerCharacter->IsAiming();
	bElimmed = PlayerCharacter->IsElimmed();
	bHoldingBomb = PlayerCharacter->isHoldingBomb();
	EquippedWeapon = PlayerCharacter->GetEuippedWeapon();
	TurningInPlace = PlayerCharacter->GetTurningInPlace();

	FRotator AimRotation = PlayerCharacter->GetBaseAimRotation();
	const FRotator MoveMentRotation = UKismetMathLibrary::MakeRotFromX(PlayerCharacter->GetVelocity());
	FRotator DeltaRote = UKismetMathLibrary::NormalizedDeltaRotator(MoveMentRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRote, DeltaSeconds, 5.f);
	YawOffset= DeltaRotation.Yaw;
	
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = PlayerCharacter->GetActorRotation();

	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw/DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = PlayerCharacter->GetAO_Yaw();
	AO_Pitch = PlayerCharacter->GetAO_Pitch();

	bUseFABRIK = PlayerCharacter->GetCombatState() != ECombatState::ECS_Reloading
		&& PlayerCharacter->GetCombatState() != ECombatState::ECS_TossGrenade
		&& PlayerCharacter->GetCombatState() != ECombatState::ECS_SwappingWeapons && !bHoldingBomb;

	if(!bUseFABRIK && PlayerCharacter->IsLocallyControlled()
		&& PlayerCharacter->GetCombatState() == ECombatState::ECS_Reloading)
	{
		bUseFABRIK = !PlayerCharacter->IsLocallyReloading();
	}
	
	if(bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && PlayerCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()
			->GetSocketTransform(FName("LeftHandSocket"), RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		PlayerCharacter->GetMesh()->
			TransformToBoneSpace(FName(TEXT("右手首")), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (PlayerCharacter && PlayerCharacter->IsLocallyControlled()
			&& bUseFABRIK)
		{
			FTransform RightHandTransform = PlayerCharacter->GetMesh()->GetSocketTransform(FName("RightHandSocket"), RTS_World);

			FRotator RightHandRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), PlayerCharacter->GetHitTarget());
			RightHandTransform.SetRotation(FQuat(RightHandRotation));

			FVector PlayerY(FRotationMatrix(RightHandTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::Y));

			FVector WeaponLocation = EquippedWeapon->GetWeaponMesh()->GetSocketLocation(FName("Root_Bone1"));
			FRotator WeaponRotation = UKismetMathLibrary::MakeRotFromXY(-PlayerY, PlayerCharacter->GetHitTarget() - WeaponLocation);

			FRotator CurrentWeaponRotation = EquippedWeapon->GetWeaponMesh()->GetComponentRotation();

			float DeltaTime = GetWorld()->GetDeltaSeconds();
			FRotator SmoothedWeaponRotation = FMath::RInterpTo(CurrentWeaponRotation, WeaponRotation, DeltaTime, 10.0f);

			EquippedWeapon->GetWeaponMesh()->SetWorldRotation(SmoothedWeaponRotation);
		}
	}
}

