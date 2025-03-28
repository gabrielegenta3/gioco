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

	TArray<AUnit*> MyUnits;

	UFUNCTION()
	void OnSniperButtonClicked();

	UFUNCTION()
	void OnBrawlerButtonClicked();

	UFUNCTION()
	void OnResetButtonClicked();

	UFUNCTION()
	void OnPassButtonClicked();

	// method to check if the unit has any enemy at attack range
	bool CanAttack();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	TArray<FVector> CurrentPath;

	int32 CurrentPathIndex;

	FTimerHandle MoveTimerHandle;

	AUnit* ControlledUnit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flags")
	bool SniperMoved;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flags")
	bool BrawlerMoved;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flags")
	bool SniperAttacked;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flags")
	bool BrawlerAttacked;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flags")
	bool SniperPlaced;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flags")
	bool BrawlerPlaced;

	// function to check in which tile a Unit could go based on their max step
	void BFSMovementRange(int32 startX, int32 startY, int32 size, int32 maxSteps, TArray<bool>& visited, AGameField* GF);

	// function to check in which tile a Unit could attack based on their range
	void BFSAttackRange(int32 startX, int32 startY, int32 size, int32 maxSteps, TArray<bool>& visited, AGameField* GF);

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
