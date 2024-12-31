// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/MyPlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "HUD/Announcement.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/PlayerHUD.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerCharacter.h"

void AMyPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	PlayerHUD = PlayerHUD==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->HealthBar &&PlayerHUD->CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health/MaxHealth;
		PlayerHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText =
			FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));

		PlayerHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AMyPlayerController::SetHUDScore(float Score)
{
	PlayerHUD  = PlayerHUD==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->ScoreAmount)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		PlayerHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
	}
}

void AMyPlayerController::SetHUDDefeats(int32 Defeats)
{
	PlayerHUD = PlayerHUD ==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->DefeatsAmount)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		PlayerHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
	}
}

void AMyPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	PlayerHUD = PlayerHUD ==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->WeaponAmmoAmount)
	{
		const FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		PlayerHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AMyPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	PlayerHUD = PlayerHUD ==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->CarriedAmmoAmount)
	{
		const FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		PlayerHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AMyPlayerController::ResetCrosshair() const
{
	if(PlayerHUD)
	{
		FHUDPackage HUDPackage;
		PlayerHUD->SetHUDPackage(HUDPackage);
	}
}

void AMyPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	PlayerHUD = PlayerHUD ==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->MatchCountdownText)
	{
		FString AmmoText;

		const int32 Minutes = FMath::FloorToInt(CountdownTime/60.f);
		const int32 Seconds = CountdownTime - Minutes*60;

		const FString CountdownText = FString::Printf(TEXT("%2d:%02d"), Minutes, Seconds);
		PlayerHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AMyPlayerController::ResetHUD()
{
	ResetCrosshair();
	if(const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetCharacter()))
	{
		const float MaxHealth = PlayerCharacter->GetMaxHealth();
		SetHUDHealth(MaxHealth, MaxHealth);
	}
}

void AMyPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetHUDTime();
	CheckTimeSync(DeltaSeconds);
	if(bInitializeCharacterOverlay)
	{
		PollInit();
	}
}

void AMyPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if(IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

float AMyPlayerController::GetServerTime()
{
	if(HasAuthority())return GetWorld()->GetTimeSeconds();
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AMyPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void AMyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPlayerController, MatchState);
}

void AMyPlayerController::HandleMatchHasStarted()
{
	PlayerHUD = PlayerHUD ==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD)
	{
		if(PlayerHUD->Announcement)
		{
			PlayerHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		PlayerHUD->AddCharacterOverlay();
	}
}

void AMyPlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void AMyPlayerController::SetHUDTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());

	if(CountdownInt!=SecondsLeft)
	{
		SetHUDMatchCountdown(MatchTime - GetServerTime());
	}
	CountdownInt = SecondsLeft;
}

void AMyPlayerController::CheckTimeSync(float DeltaSeconds)
{
	TimeSyncRunningTime += DeltaSeconds;
	if(IsLocalController() && TimeSyncRunningTime>TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AMyPlayerController::PollInit()
{
	if(CharacterOverlay==nullptr)
	{
		if(PlayerHUD && PlayerHUD->CharacterOverlay)
		{
			CharacterOverlay = PlayerHUD->CharacterOverlay;
			if(CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
				bInitializeCharacterOverlay = false;
			}
		}
	}
}

void AMyPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
                                                                float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f*RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void AMyPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PlayerHUD = Cast<APlayerHUD>(GetHUD());
	if(PlayerHUD)
	{
		PlayerHUD->AddAnnouncement();
	}
}

void AMyPlayerController::ServerCheckMatchState_Implementation()
{
	
}
