// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootGame/Public/Player/PlayerAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
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
}
