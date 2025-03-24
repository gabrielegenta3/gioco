// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "HumanPlayer.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "game_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class GIOCO_API Agame_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	Agame_PlayerController();

	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* GameInputContext;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* ClickAction;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> HUDClass;

	UPROPERTY()
	UUserWidget* HUD;

	UPROPERTY(meta = (BindWidget))
	UButton* SniperButton;

	UPROPERTY(meta = (BindWidget))
	UButton* BrawlerButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ResetButton;

	void ClickOnGrid();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
};
