
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTGAME_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	friend class APlayerCharacter;

	void EquipWeapon(class AWeapon* WeaponToEquipped);
protected:
	virtual void BeginPlay() override;
private:
	class APlayerCharacter* Character;
	AWeapon* EquippedWeapon;
public:	
	
		
};
