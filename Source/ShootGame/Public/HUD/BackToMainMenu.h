// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MultiPlayerSessionsSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "BackToMainMenu.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTGAME_API UBackToMainMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	void MenuSetup();
	void MenuTeardown();
protected:
	virtual bool Initialize() override;

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	UFUNCTION()
	void OnPlayerLeftGame();
private:
	UPROPERTY(meta=(BindWidget))
	class UButton* ReturnButton;

	UFUNCTION()
	void ReturnButtonClicked();

	UPROPERTY()
	UMultiPlayerSessionsSubsystem* MultiPlayerSessionsSubsystem;

	UPROPERTY()
	APlayerController* PlayerController;
};
