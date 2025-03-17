// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/MyPlayerController.h"

#include "Components/CombatComponent.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerState.h"
#include "GameMode/ShootGameMode.h"
#include "GameState/ShootGameState.h"
#include "HUD/Announcement.h"
#include "HUD/BackToMainMenu.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/PlayerHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerCharacter.h"
#include "PlayerController/Announcement.h"

void AMyPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	PlayerHUD = PlayerHUD==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->HealthBar && PlayerHUD->CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health/MaxHealth;
		PlayerHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText =
			FString::Printf(TEXT("%d"), FMath::CeilToInt(Health));

		PlayerHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AMyPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	PlayerHUD = PlayerHUD==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->ShieldBar && PlayerHUD->CharacterOverlay->ShieldText)
	{
		const float ShieldPercent = Shield/MaxShield;
		PlayerHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText =
			FString::Printf(TEXT("%d"), FMath::CeilToInt(Shield));

		PlayerHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
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
		bInitializeScore= true;
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
		bInitializeDefeats = true;
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
		UE_LOG(LogTemp, Log, TEXT("SetHud: %s"), *AmmoText);
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
		if(CountdownTime<1.f)
		{
			PlayerHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		
		const int32 Minutes = FMath::FloorToInt(CountdownTime/60.f);
		const int32 Seconds = CountdownTime - Minutes*60;

		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AMyPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	PlayerHUD = PlayerHUD ==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->Announcement &&
		PlayerHUD->Announcement->WarmupTime)
	{
		if(CountdownTime<1.f)
		{
			PlayerHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		
		const int32 Minutes = FMath::FloorToInt(CountdownTime/60.f);
		const int32 Seconds = CountdownTime - Minutes*60;

		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void AMyPlayerController::SetHUDGrenades(int32 Grenades)
{
	PlayerHUD = PlayerHUD ==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->MatchCountdownText)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		PlayerHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		HUDGrenades = Grenades;
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
	PollInit();

	CheckPing(DeltaSeconds);
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

void AMyPlayerController::OnMatchStateSet(FName State, bool bTeams)
{
	MatchState = State;
	
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeams);
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AMyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPlayerController, MatchState);
	DOREPLIFETIME(AMyPlayerController, bShowTeamScores);
}

void AMyPlayerController::HandleMatchHasStarted(bool bTeams)
{
	if(HasAuthority())
	{
		bShowTeamScores = bTeams;
	}
	PlayerHUD = PlayerHUD ==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD)
	{
		if(PlayerHUD->Announcement)
		{
			PlayerHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		if(HasAuthority())
		{
			if(bTeams)
			{
				InitTeamScores();
			}
			else
			{
				HideTeamScores();
			}
		}
		PlayerHUD->AddCharacterOverlay();
	}
}

void AMyPlayerController::HandleCooldown()
{
	PlayerHUD = PlayerHUD ==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;

	if(APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->bDisableGameplay = true;
		PlayerCharacter->CancelCombatComponentFireButtonPressed();
	}
	
	if(PlayerHUD)
	{
		PlayerHUD->CharacterOverlay->RemoveFromParent();
		if(PlayerHUD->Announcement)
		{
			PlayerHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			if(PlayerHUD->Announcement->AnnouncementText)
			{
				FString AnnouncementText = Announcement::NewMatchStartsIn;
				PlayerHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			}

			if(PlayerHUD->Announcement->InfoText)
			{
				if(AShootGameState* ShootGameState = Cast<AShootGameState>
					(UGameplayStatics::GetGameState(this)))
				{
					TArray<APlayerState*> TopPlayers = ShootGameState->TopScoringPlayers;
					FString InfoTextString = bShowTeamScores ?
						GetTeamsInfoText(ShootGameState) : GetInfoText(TopPlayers);
					
					PlayerHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
				}
			}
		}
	}
}


void AMyPlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AMyPlayerController::ServerReportPingStatus_Implementation(bool HighPing)
{
	HighPingDelegate.Broadcast(HighPing);
}

void AMyPlayerController::SetHUDTime()
{
	uint32 SecondsLeft = 0.f;
	float TimeLeft = 0.f;
	if(MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}
	else if(MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	else if(MatchState == MatchState::Cooldown)
	{
		TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	if(HasAuthority())
	{
		if((GameMode =
			GameMode == nullptr?
			Cast<AShootGameMode>(UGameplayStatics::GetGameMode(this)):GameMode))
		{
			SecondsLeft = FMath::CeilToInt(GameMode->GetCountdownTime() + LevelStartingTime);
		}
	}
	else
	{
		SecondsLeft = FMath::CeilToInt(TimeLeft);
	}
	
	if(CountdownInt!=SecondsLeft)
	{
		if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		else if(MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
		CountdownInt = SecondsLeft;
	}
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
				if(bInitializeHealth)
				{
					SetHUDHealth(HUDHealth, HUDMaxHealth);
				}
				if(bInitializeShield)
				{
					SetHUDShield(HUDShield, HUDMaxShield);
				}
				if(bInitializeScore)
				{
					SetHUDScore(HUDScore);
				}
				if(bInitializeDefeats)
				{
					SetHUDDefeats(HUDDefeats);
				}
				
				if(APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());
					PlayerCharacter && PlayerCharacter->GetCombat() && !bInitializeGrenades)
				{
					SetHUDGrenades(PlayerCharacter->GetCombat()->GetGrenades());
				}
			}
		}
	}
}

void AMyPlayerController::HighPingWarning()
{
	PlayerHUD = PlayerHUD==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->HighPingImage
		&&PlayerHUD->CharacterOverlay->HighPingAnimation)
	{
		PlayerHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		PlayerHUD->CharacterOverlay->PlayAnimation(PlayerHUD->CharacterOverlay->HighPingAnimation);
	}
}

void AMyPlayerController::StopHighPingWarning()
{
	PlayerHUD = PlayerHUD==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->HighPingImage
		&&PlayerHUD->CharacterOverlay->HighPingAnimation)
	{
		PlayerHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if(PlayerHUD->CharacterOverlay->
			IsAnimationPlaying(PlayerHUD->CharacterOverlay->HighPingAnimation))
		{
			PlayerHUD->CharacterOverlay->
				StopAnimation(PlayerHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

void AMyPlayerController::CheckPing(const float DeltaSeconds)
{
	HighPingRunningTime += DeltaSeconds;
	if(HighPingRunningTime>CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ?
			TObjectPtr<APlayerState>(GetPlayerState<APlayerState>()) : PlayerState;
		if(PlayerState)
		{
			if(PlayerState->GetPingInMilliseconds()>HighPingThreshold)
			{
				PingAnimationRunningTime = 0.f;
				HighPingWarning();
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	if(PlayerHUD
		&& PlayerHUD->CharacterOverlay
		&& PlayerHUD->CharacterOverlay->HighPingAnimation
		&& PlayerHUD->CharacterOverlay->
			IsAnimationPlaying(PlayerHUD->CharacterOverlay->HighPingAnimation))
	{
		PingAnimationRunningTime += DeltaSeconds;
		if(PingAnimationRunningTime>HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void AMyPlayerController::OnRep_ShowTeamScores()
{
	if(bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}

FString AMyPlayerController::GetInfoText(const TArray<APlayerState*>& PlayerStates)
{
	FString InfoTextString;
	if(PlayerStates.Num() == 0)
	{
		InfoTextString = *Announcement::ThereIsNoWinner;
	}
	else
	{
		InfoTextString = *Announcement::TheWinnerIs;
		for(auto TiedPlayer:PlayerStates)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}
	return InfoTextString;
}

FString AMyPlayerController::GetTeamsInfoText(AShootGameState* ShootGameState)
{
	if(ShootGameState == nullptr)return FString();
	FString InfoTextString;
	int32 RedTeamScore = ShootGameState->RedTeamScore;
	int32 BlueTeamScore = ShootGameState->BlueTeamScore;
	ETeam MyTeam = GetPlayerState<AMyPlayerState>()->GetTeam();
	if(RedTeamScore ==  BlueTeamScore)
	{
		InfoTextString = *Announcement::TeamTied;
	}
	else if(RedTeamScore > BlueTeamScore)
	{
		if(MyTeam == ETeam::ET_RedTeam)
		{
			InfoTextString = *Announcement::TeamWin;
		}
		else
		{
			InfoTextString = *Announcement::TeamLose;
		}
	}
	else
	{
		if(MyTeam == ETeam::ET_RedTeam)
		{
			InfoTextString = *Announcement::TeamLose;
		}
		else
		{
			InfoTextString = *Announcement::TeamWin;
		}
	}
	return InfoTextString;
}

void AMyPlayerController::ShowBackToMainMenu()
{
	if(BackToMainMenuWidget == nullptr)return;
	if(BackToMainMenu == nullptr)
	{
		BackToMainMenu = CreateWidget<UBackToMainMenu>(this, BackToMainMenuWidget);
	}
	if(BackToMainMenu)
	{
		bBackToMainMenuOpen = !bBackToMainMenuOpen;
		if(bBackToMainMenuOpen)
		{
			BackToMainMenu->MenuSetup();
		}
		else
		{
			BackToMainMenu->MenuTeardown();
		}
	}
}

void AMyPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void AMyPlayerController::HideTeamScores()
{
	PlayerHUD = PlayerHUD==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->EnemyTeamScoreText
		&& PlayerHUD->CharacterOverlay->MyTeamScoreText)
	{
		PlayerHUD->CharacterOverlay->EnemyTeamScoreText->SetText(FText());
		PlayerHUD->CharacterOverlay->MyTeamScoreText->SetText(FText());
	}
}

void AMyPlayerController::InitTeamScores()
{
	PlayerHUD = PlayerHUD==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->EnemyTeamScoreText
		&& PlayerHUD->CharacterOverlay->MyTeamScoreText)
	{
		const FString Zero("0");
		PlayerHUD->CharacterOverlay->EnemyTeamScoreText->SetText(FText::FromString(Zero));
		PlayerHUD->CharacterOverlay->MyTeamScoreText->SetText(FText::FromString(Zero));
	}
}

void AMyPlayerController::SetHUDEnemyTeamScore(int32 Score)
{
	PlayerHUD = PlayerHUD==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->EnemyTeamScoreText)
	{
		const FString ScoreText = FString::Printf(TEXT("%d"), Score);
		PlayerHUD->CharacterOverlay->EnemyTeamScoreText->SetText(FText::FromString(ScoreText));
	}	
}

void AMyPlayerController::SetHUDMyTeamScore(int32 Score)
{
	PlayerHUD = PlayerHUD==nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->MyTeamScoreText)
	{
		const FString ScoreText = FString::Printf(TEXT("%d"), Score);
		PlayerHUD->CharacterOverlay->MyTeamScoreText->SetText(FText::FromString(ScoreText));
	}	
}

void AMyPlayerController::ShowHitCrosshair()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->HitCrosshair)
	{
		PlayerHUD->CharacterOverlay->HitCrosshair->SetOpacity(1.f);
	}
}

void AMyPlayerController::HideHitCrosshair()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->HitCrosshair)
	{
		PlayerHUD->CharacterOverlay->HitCrosshair->SetOpacity(0.f);
	}
}

void AMyPlayerController::ShowHitCrosshairWithTimer()
{
	ShowHitCrosshair();

	GetWorldTimerManager().ClearTimer(HitCrosshairTimer);
	
	GetWorldTimerManager().SetTimer(
		HitCrosshairTimer, 
		this, 
		&AMyPlayerController::HideHitCrosshair, 
		HitCrosshairShowTime, 
		false
	);
}


void AMyPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	if(Attacker && Victim)
	{
		PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()):PlayerHUD;
		if(PlayerHUD)
		{
			PlayerHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}

void AMyPlayerController::ClientJoinMidgame_Implementation
	(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	CooldownTime = Cooldown;
	OnMatchStateSet(MatchState);
	PlayerHUD = Cast<APlayerHUD>(GetHUD());
	if(PlayerHUD && MatchState == MatchState::WaitingToStart)
	{
		PlayerHUD->AddAnnouncement();
	}
}

void AMyPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
                                                                float TimeServerReceivedClientRequest)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = RoundTripTime * 0.5f;
	const float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
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
	ServerCheckMatchState();
}

void AMyPlayerController::ServerCheckMatchState_Implementation()
{
	if((GameMode =
			GameMode == nullptr?Cast<AShootGameMode>(UGameplayStatics::GetGameMode(this)):GameMode))
	{
		WarmupTime = GameMode->WarmUpTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		CooldownTime = GameMode->CooldownTime;
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, LevelStartingTime, CooldownTime);

		
	}
	
}
