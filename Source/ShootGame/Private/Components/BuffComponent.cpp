

#include "Components/BuffComponent.h"
#include "Player/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	HealRampUp(DeltaTime);
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if(bHealing == false || Character == nullptr || Character->IsElimmed()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth()+HealThisFrame,
				0, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;

	if(AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::Heal(float HealingAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealingAmount/HealingTime;
	AmountToHeal += HealingAmount;

	if(Character)
	{
		Character->PlaySound(ECharacterSoundType::EST_HealSound);
	}
}

void UBuffComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed)
{
	InitBaseSpeed = BaseSpeed;
	InitCrouchSpeed = CrouchSpeed;
}


void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if(Character == nullptr)return;
	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&ThisClass::ResetSpeed,
		BuffTime);
	
	MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
	
	Character->PlaySound(ECharacterSoundType::EST_SpeedSound);

}

void UBuffComponent::ResetSpeed()
{
	MulticastSpeedBuff(InitBaseSpeed, InitCrouchSpeed);
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	}
}

void UBuffComponent::ResetJump()
{
	if(Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	}
	MulticastJumpBuff(InitialJumpVelocity);
}

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if(Character == nullptr)return;
	Character->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&ThisClass::ResetJump,
		BuffTime);

	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
	}
	MulticastJumpBuff(BuffJumpVelocity);

	Character->PlaySound(ECharacterSoundType::EST_JumpSound);

}

void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if(Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}

void UBuffComponent::ReplenishShield(const float ShieldAmount) const
{
	if(Character==nullptr || Character->IsElimmed())return;
	Character->SetShield
	(FMath::Clamp(Character->GetShield()+ShieldAmount,
		0.f,
		Character->GetMaxShield()));
	Character->UpdateHUDShield();
	Character->PlaySound(ECharacterSoundType::EST_ShieldSound);
}

void UBuffComponent::StopHealing()
{
	bHealing = false;
	AmountToHeal = 0.f;
	HealingRate = 0.f;
}
