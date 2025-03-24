// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModality.h"
#include "GameField.h"
#include "EngineUtils.h"
#include "HumanPlayer.h"
#include "RandomPlayer.h"
#include "HUDWidget.h"
#include "game_PlayerController.h"
#include "Camera/CameraActor.h"

AGameModality::AGameModality()
{
	FieldSize = 25;
	GameFieldClass = AGameField::StaticClass();
}

void AGameModality::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("AGameModality::BeginPlay chiamato!"));
	Super::BeginPlay();
	IsGameOver = false;
	MoveCounter = 0;
	AHumanPlayer* HumanPlayer = GetWorld()->GetFirstPlayerController()->GetPawn<AHumanPlayer>();
	if (!IsValid(HumanPlayer))
	{
		UE_LOG(LogTemp, Error, TEXT("No player pawn of type '%s' was found."), *AHumanPlayer::StaticClass()->GetName());
		return;
	}

	if (GameFieldClass != nullptr) 
	{
		GameField = GetWorld()->SpawnActor<AGameField>(GameFieldClass);
		GameField->Size = FieldSize;
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("GameField is null"));
	}

	/*if (HUDWidgetClass)
	{
		// Prendi il PlayerController
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			// Crea il widget e aggiungilo allo schermo
			HUDInstance = CreateWidget<UHUDWidget>(PC, HUDWidgetClass);
			if (HUDInstance)
			{
				HUDInstance->AddToViewport();
			}
		} 
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Player controller non valido"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HUDWidget non caricato"));
	}*/
	

	// set camera position
	float CameraPosX = ((GameField->TileSize * FieldSize) + ((FieldSize - 1) * GameField->TileSize * GameField->CellPadding)) * 0.5f;
	float Zposition = 2600.0f;
	FVector CameraPos(CameraPosX, CameraPosX, Zposition);

	FRotator CameraRotation = FRotator(-90.0f, -90.0f, 0.0f);

	HumanPlayer->SetActorLocationAndRotation(CameraPos, CameraRotation);


	// Human player = 0
	Players.Add(HumanPlayer);
	// Random Player
	auto* AI = GetWorld()->SpawnActor<ARandomPlayer>(FVector(), FRotator());

	// MiniMax Player
	//auto* AI = GetWorld()->SpawnActor<AMinimaxPlayer>(FVector(), FRotator());

	// AI player = 1
	Players.Add(AI);

	this->ChoosePlayerAndStartGame();
}

void AGameModality::ChoosePlayerAndStartGame()
{
	CurrentPlayer = FMath::RandRange(0, Players.Num() - 1);

	for (int32 IndexI = 0; IndexI < Players.Num(); IndexI++)
	{
		Players[IndexI]->PlayerNumber = IndexI;
	}
	MoveCounter += 1;
	Players[CurrentPlayer]->OnTurn();
}

void AGameModality::SpawnCellUnit(int32 PlayerNumber, const FVector& SpawnPosition, const EPawnType Type)
{
	AGameField* FoundField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	FVector2D XYPosition =  FoundField->GetXYPositionByRelativeLocation(SpawnPosition);
	AUnit* Unit = GetWorld()->SpawnActor<AUnit>(UnitClass, SpawnPosition, FRotator::ZeroRotator);
	if (!Unit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Impossibile spawnare ATile a %s"), *SpawnPosition.ToString());
		return;
	}
	Unit->Init(Type, PlayerNumber, XYPosition); // the tile is an obstacle if Obstacles[i * Size + j] is true
	const float TileScale = FoundField->TileSize / 100.f;
	const float Zscaling = 0.2f;
	Unit->SetActorScale3D(FVector(TileScale, TileScale, Zscaling));
	
}

int32 AGameModality::GetNextPlayer(int32 Player)
{
	return int32();
}

void AGameModality::TurnNextPlayer()
{
}
