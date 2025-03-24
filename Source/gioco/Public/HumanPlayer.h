// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerInterface.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameField.h"
#include "GameModality.h"
#include "game_GameInstance.h"
#include "Unit.h"
#include "Components/Button.h"
#include "Blueprint/UserWidget.h"
#include "HumanPlayer.generated.h"

UCLASS()
class GIOCO_API AHumanPlayer : public APawn, public IPlayerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AHumanPlayer();

	// camera component attached to player pawn
	UCameraComponent* Camera;

	// game instance reference 
	Ugame_GameInstance* GameInstance;

	UFUNCTION()
	void OnSniperButtonClicked();

	UFUNCTION()
	void OnBrawlerButtonClicked();

	UFUNCTION()
	void OnResetButtonClicked();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	bool IsMyTurn = false;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnTurn() override;
	virtual void OnWin() override;
	virtual void OnLose() override;

	// called on left mouse click
	UFUNCTION()
	void OnClick();

};
