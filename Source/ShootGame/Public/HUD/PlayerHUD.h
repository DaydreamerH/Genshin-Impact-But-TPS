// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Player/MyPlayerState.h"
#include "PlayerHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairsCenter = nullptr;
	UTexture2D* CrosshairsLeft = nullptr;
	UTexture2D* CrosshairsRight = nullptr;
	UTexture2D* CrosshairsTop = nullptr;
	UTexture2D* CrosshairsBottom = nullptr;
	float CrosshairSpread;
};

UCLASS()
class SHOOTGAME_API APlayerHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category="Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;
	UPROPERTY(EditAnywhere, Category="Player Stats")
	TSubclassOf<class UUserWidget> AnnouncementClass;
	
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	UPROPERTY()
	class UAnnouncement* Announcement;
	
	void AddCharacterOverlay();
	void AddAnnouncement();
	void AddElimAnnouncement(AMyPlayerState* Attacker, AMyPlayerState* Victim);
protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	class APlayerController* OwingPlayer;
	
	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2d ViewportCenter, FVector2d Spread);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UElimAnnouncement> ElimAnnouncementClass;

	UPROPERTY(EditAnywhere)
	float ElimAnnouncementTime = 3.f;

	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove);

	UPROPERTY()
	TArray<UElimAnnouncement*> ElimMsgs;
public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package){ HUDPackage = Package; }
	void RemoveCrosshair();
};
