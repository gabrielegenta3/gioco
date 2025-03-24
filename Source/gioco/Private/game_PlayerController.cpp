// Fill out your copyright notice in the Description page of Project Settings.



#include "game_PlayerController.h"



Agame_PlayerController::Agame_PlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
}

void Agame_PlayerController::ClickOnGrid()
{
	const auto HumanPlayer = Cast<AHumanPlayer>(GetPawn());
	if (IsValid(HumanPlayer))
	{
		HumanPlayer->OnClick();
	}
}

void Agame_PlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(GameInputContext, 0);
	}

	if (HUD) // Se il widget esiste già, rimuovilo prima di crearne un altro
	{
		HUD->RemoveFromParent();
		HUD = nullptr;
	}

	if (HUDClass)
	{
		HUD = CreateWidget<UUserWidget>(this, HUDClass);
		if (HUD)
		{
			//HUD->AddToViewport();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Errore nella creazione dell'HUD!"));
		}
	}
	
}

void Agame_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Triggered, this, &Agame_PlayerController::ClickOnGrid);
	}
}
