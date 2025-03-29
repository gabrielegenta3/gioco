// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModality.h"
#include "GameFramework/Pawn.h"
#include "PlayerInterface.h"
#include "game_GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "RandomPlayer.generated.h"

UCLASS()
class GIOCO_API ARandomPlayer : public APawn, public IPlayerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ARandomPlayer();

	// game instance reference
	Ugame_GameInstance* GameInstance;
	
	TArray<AUnit*> MyUnits;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flags")
	bool bIsSmart;  //^=.=^

	FTimerHandle TimerHandle1;

	FTimerHandle TimerHandle2;

	FTimerHandle BrawlerHighlightTimerHandler;

	FTimerHandle SniperHighlightTimerHandler;

	FTimerHandle BrawlerMoveTimerHandler;

	FTimerHandle SniperMoveTimerHandler;

	FTimerHandle BrawlerAttackTimerHandler;

	FTimerHandle SniperAttackTimerHandler;

	
	// function to check in which tile a Unit could go based on their max step
	void BFSMovementRange(int32 startX, int32 startY, int32 size, int32 maxSteps, TArray<bool>& visited, AGameField* GF);

	// function to check in which tile a Unit could attack based on their range
	void BFSAttackRange(int32 startX, int32 startY, int32 size, int32 maxSteps, TArray<bool>& visited, AGameField* GF);

	void HighlightAndMoveBrawler();

	void HighlightAndMoveSniper();

	void MoveBrawler();

	void MoveSniper();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnTurn() override;
	virtual void OnWin() override;
	virtual void OnLose() override;
};
