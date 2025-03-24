// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "game_GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class GIOCO_API Ugame_GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	// score value for human player
	UPROPERTY(EditAnywhere)
	int32 ScoreHumanPlayer = 0;

	// score value for AI player
	UPROPERTY(EditAnywhere)
	int32 ScoreAiPlayer = 0;

	// message to show every turn
	UPROPERTY(EditAnywhere)
	FString CurrentTurnMessage = "Current Player";

	// flag to check if SniperButton is clicked
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flags")
	bool bSniperButtonClicked = false;

	// flag to check if BrawlerButton is clicked
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flags")
	bool bBrawlerButtonClicked = false;

	// increment the score for human player
	void IncrementScoreHumanPlayer();

	// increment the score for AI player
	void IncrementScoreAiPlayer();

	// get the score for human player
	UFUNCTION(BlueprintCallable)
	int32 GetScoreHumanPlayer();

	// get the score for AI player
	UFUNCTION(BlueprintCallable)
	int32 GetScoreAiPlayer();

	// get the current turn message
	UFUNCTION(BlueprintCallable)
	FString GetTurnMessage();

	// set the turn message
	UFUNCTION(BlueprintCallable)
	void SetTurnMessage(FString Message);
};
