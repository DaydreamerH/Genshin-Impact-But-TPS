// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/PlayerHUD.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "HUD/Announcement.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/ElimAnnouncement.h"

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
}

void APlayerHUD::AddCharacterOverlay()
{
	if(APlayerController* PlayerController = GetOwningPlayerController();
		PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
		if(CharacterOverlay->HitCrosshair)
		{
			CharacterOverlay->HitCrosshair->SetOpacity(0.f);
		}
	}
}

void APlayerHUD::AddAnnouncement()
{
	if(APlayerController* PlayerController
		= GetOwningPlayerController(); PlayerController && AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		Announcement->AddToViewport();
	}
}

void APlayerHUD::AddElimAnnouncement(AMyPlayerState* Attacker, AMyPlayerState* Victim)
{
	OwingPlayer = OwingPlayer == nullptr?GetOwningPlayerController():OwingPlayer;
	if(OwingPlayer && ElimAnnouncementClass)
	{
		if(UElimAnnouncement* ElimAnnouncement
			= CreateWidget<UElimAnnouncement>(OwingPlayer, ElimAnnouncementClass))
		{
			ElimAnnouncement->SetElimAnnouncementText(Attacker, Victim);
			ElimAnnouncement->AddToViewport();

			for(auto Msg:ElimMsgs)
			{
				if(Msg && Msg->AnnouncementBox)
				{
					if(UCanvasPanelSlot* CanvasPanelSlot =
						UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox))
					{
						const FVector2d Position = CanvasPanelSlot->GetPosition();
						const FVector2d NewPosition(
							CanvasPanelSlot->GetPosition().X,
							Position.Y + CanvasPanelSlot->GetSize().Y);
						CanvasPanelSlot->SetPosition(NewPosition);
					}
					
				}
			}

			
			
			ElimMsgs.Add(ElimAnnouncement);
			FTimerHandle ElimMsgTimer;
			FTimerDelegate ElimMsgDelegate;
			ElimMsgDelegate.BindUFunction(this,
				FName("ElimAnnouncementTimerFinished"),
				ElimAnnouncement);
			GetWorldTimerManager().SetTimer(ElimMsgTimer, ElimMsgDelegate, ElimAnnouncementTime, false);
		}
	}
}

void APlayerHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
	if(MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
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


