
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTGAME_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class APlayerCharacter;
protected:
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);
private:
	UPROPERTY()
	APlayerCharacter* Character;
	
	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;

	FTimerHandle SpeedBuffTimer;
	void ResetSpeed();
	float InitBaseSpeed;
	float InitCrouchSpeed;
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);
	
	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpVelocity;
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Heal(float HealingAmount, float HealingTime);
	
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void SetInitialSpeed(float BaseSpeed, float CrouchSpeed);
	
	void BuffJump(float BuffJumpVelocity, float BuffTime);
	void SetInitialJumpVelocity(float Velocity);

	void ReplenishShield(float ShieldAmount) const;
};
