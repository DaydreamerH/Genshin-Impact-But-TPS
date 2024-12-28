// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/PlayerHUD.h"

#include "Blueprint/UserWidget.h"
#include "HUD/CharacterOverlay.h"

void APlayerHUD::DrawHUD()
{
	Super::DrawHUD();

	
	FVector2d ViewportSize;
	if(GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2d ViewportCenter(ViewportSize.X/2.f, ViewportSize.Y/2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
		
		if(HUDPackage.CrosshairsCenter)
		{
			FVector2d Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread);
		}
		if(HUDPackage.CrosshairsRight)
		{
			FVector2d Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread);
		}
		if(HUDPackage.CrosshairsTop)
		{
			FVector2d Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread);
		}
		if(HUDPackage.CrosshairsLeft)
		{
			FVector2d Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread);
		}
		if(HUDPackage.CrosshairsBottom)
		{
			FVector2d Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread);
		}
	}
}

void APlayerHUD::BeginPlay()
{
	Super::BeginPlay();

	AddCharacterOverlay();
}

void APlayerHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if(PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void APlayerHUD::DrawCrosshair(UTexture2D* Texture, FVector2d ViewportCenter, FVector2d Spread)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();

	const FVector2d
		TextureDrawpoint(
			ViewportCenter.X - (TextureWidth/2.f) + Spread.X,
			ViewportCenter.Y - (TextureHeight/2.f) + Spread.Y
			);

	DrawTexture(
		Texture,
		TextureDrawpoint.X,
		TextureDrawpoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,0.f,
		1.f,1.f,
		FLinearColor::White
	);
}
