#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/Weapon.h"
#include "GenericSpawnPoint.generated.h"

UCLASS()
class SHOOTGAME_API AGenericSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	AGenericSpawnPoint();
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	void SpawnWeapon();

	void SpawnWeaponTimerFinished();

	void StartSpawnWeaponTimer(AActor* DestroyedActor);

	UFUNCTION()
	void OnWeaponStateChanged(EWeaponState WeaponState);

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TArray<TSubclassOf<class AWeapon>> WeaponClasses;

	UPROPERTY()
	AWeapon* SpawnedWeapon = nullptr;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float SpawnInterval = 5.0f;
	
	FTimerHandle SpawnTimerHandle;
};
