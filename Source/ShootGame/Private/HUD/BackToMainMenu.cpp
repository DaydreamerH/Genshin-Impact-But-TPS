// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BackToMainMenu.h"

#include "Components/Button.h"
#include "GameFramework/GameModeBase.h"
#include "Player/PlayerCharacter.h"

void UBackToMainMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	if(const UWorld* World = GetWorld())
	{
		if((PlayerController = PlayerController == nullptr ?
				World->GetFirstPlayerController():PlayerController))
		{
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	if(ReturnButton && !ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.AddDynamic(this, &ThisClass::ReturnButtonClicked);
	}
	if(const UGameInstance* GameInstance = GetGameInstance())
	{
		MultiPlayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiPlayerSessionsSubsystem>();
		if(MultiPlayerSessionsSubsystem)
		{
			MultiPlayerSessionsSubsystem->
				MultiPlayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		}
	}
}

bool UBackToMainMenu::Initialize()
{
	if(!Super::Initialize())
	{
		return false;
	}
	return true;
}

void UBackToMainMenu::OnDestroySession(bool bWasSuccessful)
{
	if(!bWasSuccessful)
	{
		ReturnButton->SetIsEnabled(true);
		return;
	}
	if(const UWorld* World = GetWorld())
	{
		if(AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>())
		{
			GameMode->ReturnToMainMenuHost();
		}
		else if((PlayerController = PlayerController == nullptr ?
				World->GetFirstPlayerController():PlayerController))
		{
			PlayerController->ClientReturnToMainMenuWithTextReason(FText());
		}
	}
}

void UBackToMainMenu::MenuTeardown()
{
	if(const UWorld* World = GetWorld())
	{
		if((PlayerController = PlayerController == nullptr ?
				World->GetFirstPlayerController():PlayerController))
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	if(ReturnButton && ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.RemoveDynamic(this, &ThisClass::ReturnButtonClicked);
	}
	if(MultiPlayerSessionsSubsystem &&
		MultiPlayerSessionsSubsystem->MultiPlayerOnDestroySessionComplete.IsBound())
	{
		MultiPlayerSessionsSubsystem->
			MultiPlayerOnDestroySessionComplete.RemoveDynamic(this, &ThisClass::OnDestroySession);
	}
	RemoveFromParent();
}

void UBackToMainMenu::ReturnButtonClicked()
{
	ReturnButton->SetIsEnabled(false);

	if(const UWorld* World = GetWorld())
	{
		if(const APlayerController* FirstPlayerController = World->GetFirstPlayerController())
		{
			if(APlayerCharacter* PlayerCharacter =
				Cast<APlayerCharacter>(FirstPlayerController->GetPawn()))
			{
				PlayerCharacter->ServerLeaveGame();
				PlayerCharacter->OnLeftGame.AddDynamic(this, &ThisClass::OnPlayerLeftGame);
			}
			else
			{
				ReturnButton->SetIsEnabled(true);
			}
		}
	}
}

void UBackToMainMenu::OnPlayerLeftGame()
{
	if(MultiPlayerSessionsSubsystem)
	{
		MultiPlayerSessionsSubsystem->DestroySession();
	}
}