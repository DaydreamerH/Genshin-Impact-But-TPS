// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameState/ShootGameState.h"
#include "Player/MyPlayerState.h"
#include "MyPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bHighPing);
/**
 * 
 */
UCLASS()
class SHOOTGAME_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Sheild, float MaxSheild);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void ResetCrosshair() const;
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);
	void ResetHUD();
	virtual void Tick(float DeltaSeconds) override;
	virtual void ReceivedPlayer() override;
	virtual float GetServerTime();
	void OnMatchStateSet(FName State, bool bTeams = false);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void HandleMatchHasStarted(bool bTeams = false);
	void HandleCooldown();

	float SingleTripTime = 0.f;
	
	FHighPingDelegate HighPingDelegate;

	void ShowBackToMainMenu();

	void BroadcastElim(AMyPlayerState* Attacker, AMyPlayerState* Victim);

	void HideTeamScores();
	void InitTeamScores();
	void SetHUDEnemyTeamScore(int32 Score);
	void SetHUDMyTeamScore(int32 Score);
	void ShowHitCrosshair();
	void HideHitCrosshair();
	void ShowHitCrosshairWithTimer();
	void ShowHitCrosshairHandle();
	UFUNCTION(Client, Reliable)
	void ClientShowHitCrosshairHandle();

	void RemoveCrosshair() const;
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere, Category=Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

	void CheckTimeSync(float DeltaSeconds);

	void PollInit();

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame
		(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown);

	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaSeconds);

	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(AMyPlayerState* Attacker, AMyPlayerState* Victim);

	UPROPERTY(ReplicatedUsing=OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION()
	void OnRep_ShowTeamScores();

	static FString GetInfoText(const TArray<APlayerState*>& PlayerStates);
	FString GetTeamsInfoText(const AShootGameState* ShootGameState) const;
private:
	UPROPERTY(EditAnywhere, Category=HUD)
	TSubclassOf<class UUserWidget> BackToMainMenuWidget;

	UPROPERTY()
	class UBackToMainMenu* BackToMainMenu;

	bool bBackToMainMenuOpen = false;
	
	UPROPERTY()
	class APlayerHUD* PlayerHUD;

	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	float LevelStartingTime = 0.f;
	
	uint32 CountdownInt = 0.f;

	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	
	float HUDHealth;
	bool bInitializeHealth = false;
	float HUDMaxHealth;
	float HUDShield;
	bool bInitializeShield = false;
	float HUDMaxShield;
	float HUDScore;
	bool bInitializeScore = false;
	int32 HUDDefeats;
	bool bInitializeDefeats;
	int32 HUDGrenades = false;
	bool bInitializeGrenades = false;
	
	UPROPERTY()
	class AShootGameMode* GameMode;

	float HighPingRunningTime = 0.f;
	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;
	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;
	
	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool HighPing);
	
	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 100.f;
	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HitCrosshairShowTime = 1.5f;
	FTimerHandle HitCrosshairTimer;
};