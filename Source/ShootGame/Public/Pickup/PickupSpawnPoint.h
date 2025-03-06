#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class SHOOTGAME_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickup>> PickupClasses;

	void SpawnPickup();
	void SpawnPickupTimerFinished();
	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);
	
	UPROPERTY()
	APickup* SpawnedPickup = nullptr;
private:
	FTimerHandle SpawnPickupTimer;
	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMin = 30.f;
	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMax = 60.f;
	
};
