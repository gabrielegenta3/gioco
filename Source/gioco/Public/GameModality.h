// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PlayerInterface.h"
#include "GameField.h"
#include "GameFramework/GameMode.h"
#include "Camera/CameraActor.h"
#include "Unit.h"
#include "Kismet/GameplayStatics.h"
#include "GameModality.generated.h"



/**
 * 
 */
UCLASS()
class GIOCO_API AGameModality : public AGameModeBase
{
	GENERATED_BODY()

public:
	// track if the game is over
	bool IsGameOver;
	// array of players interfaces
	TArray<IPlayerInterface*> Players;

	int32 CurrentPlayer;

	// number of moves
	int32 MoveCounter;

	// reference to the camera actor
	ACameraActor* CameraActor;

	// TSubclassOf is a template class that allows you to store a reference to a UClass object
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGameField> GameFieldClass;

	// field size
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 FieldSize;

	//reference to a GameField object
	UPROPERTY(VisibleAnywhere)
	AGameField* GameField;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AUnit> UnitClass;

	AGameModality();

	// called when the game starts or when spawned
	virtual void BeginPlay() override;

	// called at the start of the game
	void ChoosePlayerAndStartGame();

	// set the cell pawn
	AUnit* SpawnCellUnit(int32 PlayerNumber, const FVector& SpawnPosition, const EPawnType Type);

	// get the next player index
	int32 GetNextPlayer(int32 Player);

	// called at the end of the game turn
	void TurnNextPlayer();


};
