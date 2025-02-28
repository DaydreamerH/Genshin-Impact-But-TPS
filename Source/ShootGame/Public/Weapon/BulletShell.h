
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletShell.generated.h"

UCLASS()
class SHOOTGAME_API ABulletShell : public AActor
{
	GENERATED_BODY()
	
public:	
	ABulletShell();
private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* BulletShellMesh;

	UPROPERTY(EditAnywhere)
	float ShellEjectionImpulse;

	UPROPERTY(EditAnywhere)
	class USoundCue* ShellSound;
protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnHit
		(UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);
};
