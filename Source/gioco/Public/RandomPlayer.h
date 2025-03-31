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

	// method to set the difficulty of the AI
	void SetDifficulty(bool bSmart) { bIsSmart = bSmart; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flags")
	bool bSniperMoved;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flags")
	bool bBrawlerMoved;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flags")
	bool bSniperAttacked;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flags")
	bool bBrawlerAttacked;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flags")
	bool bSniperPlaced;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flags")
	bool bBrawlerPlaced;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flags")
	bool bIsSmart;  //^=.=^

	FTimerHandle TimerHandle1;

	FTimerHandle TimerHandle2;

	FTimerHandle BrawlerHighlightTimerHandle;

	FTimerHandle SniperHighlightTimerHandle;

	FTimerHandle BrawlerMoveTimerHandle;

	FTimerHandle SniperMoveTimerHandle;

	FTimerHandle BrawlerAttackHighlightTimerHandle;

	FTimerHandle SniperAttackHighlightTimerHandle;

	FTimerHandle BrawlerAttackTimerHandle;

	FTimerHandle SniperAttackTimerHandle;

	FTimerHandle NextTurnTimerHandle;

	FTimerHandle CanAttackTimerHandle;

	float waitTime;

	// function to check in which tile a Unit could go based on their max step
	void BFSMovementRange(int32 startX, int32 startY, int32 size, int32 maxSteps, TArray<bool>& visited, AGameField* GF);

	// function to check in which tile a Unit could attack based on their range
	void BFSAttackRange(int32 startX, int32 startY, int32 size, int32 maxSteps, TArray<bool>& visited, AGameField* GF);

	void HighlightAndMoveBrawler();

	void HighlightAndMoveSniper();

	void MoveBrawler();

	void MoveSniper();

	void HighlightAndAttackBrawler();

	void HighlightAndAttackSniper();

	void AttackBrawler();

	void AttackSniper();

	int32 CountStepsBFS(const FVector2D& Start, const FVector2D& Goal, AGameField* GF);

	UFUNCTION(BlueprintCallable, Category = "Pathfinding")
	int32 AStarSearch(const FVector2D& Start, const FVector2D& Goal, TArray<FVector2D>& Path);

	FVector2D FindAStarDestination(AUnit* MovingUnit, AUnit* TargetUnit);

	FString GetCellString(const FVector2D& CellCoord);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnTurn() override;

	void ResetFlags();
};
