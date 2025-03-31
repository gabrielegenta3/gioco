// Fill out your copyright notice in the Description page of Project Settings.


#include "HumanPlayer.h"
#include "HUDWidget.h"
#include "game_PlayerController.h"
#include "RandomPlayer.h"

// function to check in which tile a Unit could go based on their max step
// function to visit all the cells of the map
void AHumanPlayer::BFSMovementRange(int32 startX, int32 startY, int32 size, int32 maxSteps, TArray<bool>& visited, AGameField* GF)
{
	TQueue<FIntPoint> queue;
	TQueue<int32> distanceQueue; // per salvare i passi correnti

	queue.Enqueue(FIntPoint(startX, startY));
	distanceQueue.Enqueue(0);

	int32 startIndex = startX * size + startY;
	visited[startIndex] = true;
	GF->TileArray[startIndex]->StaticMeshComponent->SetMaterial(0, GF->TileArray[startIndex]->GreenTile);

	while (!queue.IsEmpty())
	{
		FIntPoint current;
		queue.Dequeue(current);

		int32 dist;
		distanceQueue.Dequeue(dist);

		// if dist >= maxSteps, we dont explore anymore
		if (dist >= maxSteps)
			continue;

		// 4 direction
		static const int32 DirX[4] = { 1, -1, 0, 0 };
		static const int32 DirY[4] = { 0, 0, 1, -1 };

		for (int d = 0; d < 4; d++)
		{
			int nx = current.X + DirX[d];
			int ny = current.Y + DirY[d];
			if (nx >= 0 && nx < size && ny >= 0 && ny < size)
			{
				int32 newIndex = nx * size + ny;
				if (!visited[newIndex] && GF->TileArray[newIndex]->GetTileStatus() == ETileStatus::EMPTY)
				{
					visited[newIndex] = true;
					GF->TileArray[newIndex]->HighLight();

					queue.Enqueue(FIntPoint(nx, ny));
					distanceQueue.Enqueue(dist + 1);
				}
			}
		}
	}
}


// BFS to find attackable units
void AHumanPlayer::BFSAttackRange(int32 startX, int32 startY, int32 size, int32 maxSteps, TArray<bool>& visited, AGameField* GF)
{
	TQueue<FIntPoint> queue;
	TQueue<int32> distanceQueue; // to save current steps

	queue.Enqueue(FIntPoint(startX, startY));
	distanceQueue.Enqueue(0);

	int32 startIndex = startX * size + startY;
	visited[startIndex] = true;

	while (!queue.IsEmpty())
	{
		FIntPoint current;
		queue.Dequeue(current);

		int32 dist;
		distanceQueue.Dequeue(dist);

		// If dist >= maxSteps, do not explore further
		if (dist >= maxSteps)
			continue;

		// 4 directions
		static const int32 DirX[4] = { 1, -1, 0, 0 };
		static const int32 DirY[4] = { 0, 0, 1, -1 };

		for (int d = 0; d < 4; d++)
		{
			int nx = current.X + DirX[d];
			int ny = current.Y + DirY[d];
			if (nx >= 0 && nx < size && ny >= 0 && ny < size)
			{
				int32 newIndex = nx * size + ny;
				if (!visited[newIndex])
				{
					visited[newIndex] = true;
					if (!GF->TileArray[newIndex]->bIsObstacle && GF->TileArray[newIndex]->PlayerOwner == 2)
						GF->TileArray[newIndex]->HighLight();

					queue.Enqueue(FIntPoint(nx, ny));
					distanceQueue.Enqueue(dist + 1);
				}
			}
		}
	}
}

FString AHumanPlayer::GetCellString(const FVector2D& CellCoord)
{
	// Convert the column to a letter: 0->A, 1->B, ...
	int32 Column = static_cast<int32>(CellCoord.X);
	TCHAR ColumnLetter = 'A' + Column;

	// Convert the row to a number (add 1 to use 1-based indexing)
	int32 Row = static_cast<int32>(CellCoord.Y) + 1;

	// Combine the letter and number into a string
	return FString::Printf(TEXT("%c%d"), ColumnLetter, Row);
}



// Sets default values
AHumanPlayer::AHumanPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Set this pawn to be controlled by the lowest-numbered player
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	// create a camera component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	// set the camera as the root component
	SetRootComponent(Camera);
	// get the game instance reference
	GameInstance = Cast<Ugame_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	// set the default values
	PlayerNumber = 1;

	bSniperPlaced = false;
	bBrawlerPlaced = false;
	bSniperMoved = false;
	bBrawlerMoved = false;
	bSniperAttacked = false;
	bBrawlerAttacked = false;
	bCanPass = true;
}

void AHumanPlayer::OnSniperButtonClicked()
{
	if (bIsMyTurn)
	{
		GameInstance->bSniperButtonClicked = true;
		GameInstance->bBrawlerButtonClicked = false;
	}
}

void AHumanPlayer::OnBrawlerButtonClicked()
{
	if (bIsMyTurn)
	{
		GameInstance->bBrawlerButtonClicked = true;
		GameInstance->bSniperButtonClicked = false;
	}
}

void AHumanPlayer::OnResetButtonClicked()
{
	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	Agame_PlayerController* PlayerC = Cast<Agame_PlayerController>(UGameplayStatics::GetActorOfClass(GetWorld(), Agame_PlayerController::StaticClass()));
	
	// i have to clear all and let choose difficulty to user
	PlayerC->HUD->TurnIntoMainMenuHUD();
	PlayerC->HUD->ClearScrollBox();
	PlayerC->HUD->ClearUnitHP();


	ARandomPlayer* RandomPlayer = Cast<ARandomPlayer>(Cast<AGameModality>(GetWorld()->GetAuthGameMode())->Players[1]);
	// then i destroy all my units and empty my array and do the same to the random player
	for (AUnit* Unit : this->MyUnits)
	{
		Unit->Destroy();
	}
	this->MyUnits.Empty();

	for (AUnit* Unit : RandomPlayer->MyUnits)
	{
		Unit->Destroy();
	}
	RandomPlayer->MyUnits.Empty();

	this->ResetFlags();

	RandomPlayer->ResetFlags();

	// i reset field so i could restart with another obstacle distribution
	GameField->ResetField();

	PlayerC->HUD->UpdateUnitHP();
}

void AHumanPlayer::OnPassButtonClicked()
{
	AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
	if (bIsMyTurn && bCanPass)
	{
		GameModality->TurnNextPlayer();
	}
}

// to set IA difficulty and start
void AHumanPlayer::OnHardButtonClicked()
{
	AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
	ARandomPlayer* RandomPlayer = Cast<ARandomPlayer>(GameModality->Players[1]);
	RandomPlayer->SetDifficulty(true);

	Agame_PlayerController* PlayerC = Cast<Agame_PlayerController>(UGameplayStatics::GetActorOfClass(GetWorld(), Agame_PlayerController::StaticClass()));
	PlayerC->HUD->TurnIntoPlayHUD();

	GameModality->ChoosePlayerAndStartGame();
}

void AHumanPlayer::OnEasyButtonClicked()
{
	AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
	ARandomPlayer* RandomPlayer = Cast<ARandomPlayer>(GameModality->Players[1]);
	RandomPlayer->SetDifficulty(false);

	Agame_PlayerController* PlayerC = Cast<Agame_PlayerController>(UGameplayStatics::GetActorOfClass(GetWorld(), Agame_PlayerController::StaticClass()));
	PlayerC->HUD->TurnIntoPlayHUD();

	GameModality->ChoosePlayerAndStartGame();
}



// Called when the game starts or when spawned
void AHumanPlayer::BeginPlay()
{
	Super::BeginPlay();
	Agame_PlayerController* PlayerC = Cast<Agame_PlayerController>(UGameplayStatics::GetActorOfClass(GetWorld(), Agame_PlayerController::StaticClass()));
	PlayerC->HUD->HidePassButton();
}

// Called every frame
void AHumanPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AHumanPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AHumanPlayer::OnTurn()
{
	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	if (GameField)
	{
		GameField->UnHighLight();
	}
	bIsMyTurn = true;
	bSniperAttacked = false;
	bBrawlerAttacked = false;
	bSniperMoved = false;
	bBrawlerMoved = false;
	
	AUnit* Brawler = nullptr;
	AUnit* Sniper = nullptr;
	for (AUnit* Unit : MyUnits)
	{
		if (Unit->PawnType == EPawnType::BRAWLER)
		{
			Brawler = Unit;
		}
		else
		{
			Sniper = Unit;
		}
	}

	if (!Sniper)
	{
		bSniperAttacked = true;
		bSniperMoved = true;
	}

	if (!Brawler)
	{
		bBrawlerAttacked = true;
		bBrawlerMoved = true;
	}

	Agame_PlayerController* PlayerC = Cast<Agame_PlayerController>(UGameplayStatics::GetActorOfClass(GetWorld(), Agame_PlayerController::StaticClass()));
	PlayerC->HUD->SetTurnText(1);
}

void AHumanPlayer::OnClick()
{
	// Structure containing information about the hit result
	FHitResult Hit = FHitResult(ForceInit);
	// GetHitResultUnderCursor function sends a ray from the mouse cursor on the screen and returns the hit result
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_Pawn, true, Hit);
	if (Hit.bBlockingHit && bIsMyTurn && GameInstance->bSniperButtonClicked)  // if sniper button is clicked and the player clicked on a valid tile he spawns his sniper
	{
		if (ATile* CurrTile = Cast<ATile>(Hit.GetActor()))
		{
			// check if the tile is empty
			if (CurrTile->GetTileStatus() == ETileStatus::EMPTY)
			{
				CurrTile->SetTileStatus(1, ETileStatus::OCCUPIED);
				FVector SpawnPosition = CurrTile->GetActorLocation();
				SpawnPosition.Z += 1;

				AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
				AUnit* Unit = GameModality->SpawnCellUnit(1, SpawnPosition, EPawnType::SNIPER);
				if (Unit)
				{
					MyUnits.Add(Unit);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Null Unit"));
				}

				Agame_PlayerController* PlayerC = Cast<Agame_PlayerController>(UGameplayStatics::GetActorOfClass(GetWorld(), Agame_PlayerController::StaticClass()));
				PlayerC->HUD->HideSniperButton();

				AGameField* FoundField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
				FVector2D XYPosition = FoundField->GetXYPositionByRelativeLocation(SpawnPosition);

				GameInstance->bSniperButtonClicked = false;
				bSniperPlaced = true;

				if (bBrawlerPlaced && bSniperPlaced)
				{
					PlayerC->HUD->ShowPassButton();
				}

				if (PlayerC && PlayerC->HUD)
				{
					PlayerC->HUD->UpdateUnitHP();
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Player controller or HUD is null"));
				}

				GameModality->TurnNextPlayer();
			}
		}
	} 
	else if (Hit.bBlockingHit && bIsMyTurn && GameInstance->bBrawlerButtonClicked)  // if brawler button is clicked and the player clicked on a valid tile he spawns his brawler
	{
		if (ATile* CurrTile = Cast<ATile>(Hit.GetActor()))
		{
			// check if the tile is empty
			if (CurrTile->GetTileStatus() == ETileStatus::EMPTY)
			{
				CurrTile->SetTileStatus(1, ETileStatus::OCCUPIED);
				FVector SpawnPosition = CurrTile->GetActorLocation();
				SpawnPosition.Z += 1;

				AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
				AUnit* Unit = GameModality->SpawnCellUnit(1, SpawnPosition, EPawnType::BRAWLER);
				if (Unit)
				{
					MyUnits.Add(Unit);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Null Unit"));
				}
				

				Agame_PlayerController* PlayerC = Cast<Agame_PlayerController>(UGameplayStatics::GetActorOfClass(GetWorld(), Agame_PlayerController::StaticClass()));
				PlayerC->HUD->HideBrawlerButton();

				AGameField* FoundField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
				FVector2D XYPosition = FoundField->GetXYPositionByRelativeLocation(SpawnPosition);

				GameInstance->bBrawlerButtonClicked = false;
				bBrawlerPlaced = true;

				if (PlayerC && PlayerC->HUD)
				{
					PlayerC->HUD->UpdateUnitHP();
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Player controller or HUD is null"));
				}

				if (bBrawlerPlaced && bSniperPlaced)
				{
					PlayerC->HUD->ShowPassButton();
				}

				GameModality->TurnNextPlayer();
			}
		}
	}
	else if (Hit.bBlockingHit && bIsMyTurn && GameInstance->bIsUnitClicked && bSniperPlaced && bBrawlerPlaced)  // if player already clicked on a unit you can click another unit to attack him if you can, click your unit to 
	{																		// unhighlight tiles, click your other unit to highlight. You could also click another tile or unit but it does nothing obv
		if (AUnit* CurrUnit = Cast<AUnit>(Hit.GetActor())) {
			AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));

			if (CurrUnit == GameInstance->SelectedUnit && GameField)	//  unhighlight
			{
				GameInstance->SelectedUnit = nullptr;
				GameField->UnHighLight();
				GameInstance->bIsUnitClicked = false;
			}
			else if (CurrUnit->PlayerNumber == 1 && GameField)  // highlight other
			{
				GameInstance->SelectedUnit = CurrUnit;
				GameField->UnHighLight();

				FVector2D XYPosition = CurrUnit->Position;
				TArray<bool> Visited;

				if ((CurrUnit->PawnType == EPawnType::SNIPER && !bSniperMoved) || (CurrUnit->PawnType == EPawnType::BRAWLER && !bBrawlerMoved))
				{
					Visited.Init(false, GameField->Size * GameField->Size);
					BFSMovementRange(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y), GameField->Size, CurrUnit->MovementRange, Visited, GameField);
					Visited.Init(false, GameField->Size * GameField->Size);
					BFSAttackRange(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y), GameField->Size, CurrUnit->AttackRange, Visited, GameField);
				}
				else if ((CurrUnit->PawnType == EPawnType::SNIPER && !bSniperAttacked) || (CurrUnit->PawnType == EPawnType::BRAWLER && !bBrawlerAttacked))
				{
					Visited.Init(false, GameField->Size * GameField->Size);
					BFSAttackRange(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y), GameField->Size, CurrUnit->AttackRange, Visited, GameField);
				}
				else
				{
					GameInstance->SelectedUnit = nullptr;
					GameInstance->bIsUnitClicked = false;
				}
					

			}
			else if (CurrUnit->PlayerNumber == 2 && GameField)  // attack unit
			{
				if (GameField->TileArray[CurrUnit->Position.X * GameField->Size + CurrUnit->Position.Y]->bIsRed)
				{
					GameInstance->SelectedUnit->Attack(CurrUnit);
					GameField->UnHighLight();

					if (GameInstance->SelectedUnit->PawnType == EPawnType::BRAWLER)
					{
						bBrawlerAttacked = true;
						bBrawlerMoved = true;
					}
					else if (GameInstance->SelectedUnit->PawnType == EPawnType::SNIPER)
					{
						bSniperAttacked = true;
						bSniperMoved = true;
					}


					GameInstance->SelectedUnit = nullptr;
					GameInstance->bIsUnitClicked = false;

					AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
					if (GameModality->CheckWin())
					{
						return;
					}

					if (bSniperMoved && bBrawlerMoved)
					{
						if ((bSniperAttacked && bBrawlerAttacked) || !this->CanAttack())
						{
							GameModality->TurnNextPlayer();
						}
					}
				}
			}
			else if (!GameField)
			{
				UE_LOG(LogTemp, Warning, TEXT("Couldn't cast the GameField"));
			}
		}
		else if(ATile* CurrTile = Cast<ATile>(Hit.GetActor()))  // if you click an highlighted tile you can move
		{
			if (CurrTile->bIsGreen)
			{
				FVector Destination = CurrTile->GetActorLocation();
				if (GameInstance->SelectedUnit)
				{
					AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
					if (GameField)
					{
						GameField->TileArray[GameInstance->SelectedUnit->Position.X * GameField->Size + GameInstance->SelectedUnit->Position.Y]->SetTileStatus(-1, ETileStatus::EMPTY);
						FVector2D Position = GameField->GetXYPositionByRelativeLocation(Destination);

						FString UnitID = (GameInstance->SelectedUnit->PawnType == EPawnType::BRAWLER) ? TEXT("B") : TEXT("S");
						FString PlayerID = TEXT("HP");
						FString OriginCell = GetCellString(GameInstance->SelectedUnit->Position);
						FString DestinationCell = GetCellString(Position);
						
						Agame_PlayerController* PlayerC = Cast<Agame_PlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
						PlayerC->HUD->AddTextToScrollBox(FString::Printf(TEXT("%s: %s %s -> %s"), *PlayerID, *UnitID, *OriginCell, *DestinationCell));

						GameInstance->SelectedUnit->FindPathAndMove(Destination, GameField);
						GameField->UnHighLight();
						GameField->TileArray[Position.X * GameField->Size + Position.Y]->SetTileStatus(1, ETileStatus::OCCUPIED);

						if (GameInstance->SelectedUnit->PawnType == EPawnType::BRAWLER && !bBrawlerMoved)
						{
							bBrawlerMoved = true;
						}
						else if (GameInstance->SelectedUnit->PawnType == EPawnType::SNIPER && !bSniperMoved)
						{
							bSniperMoved = true;
						}

						GameInstance->SelectedUnit = nullptr;
						GameInstance->bIsUnitClicked = false;
						bCanPass = false;
						GetWorldTimerManager().SetTimer(MoveTimerHandle, [&]()
							{
								bCanPass = true;
								if (bSniperMoved && bBrawlerMoved)
								{
									if ((bSniperAttacked && bBrawlerAttacked) || !this->CanAttack())
									{
										AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
										GameModality->TurnNextPlayer();
									}
								}
							}, 2, false);
						
					}
				}
			}
		}
	}
	else if (Hit.bBlockingHit && bIsMyTurn && !GameInstance->bIsMoving && bSniperPlaced && bBrawlerPlaced)  // if nothing is clicked you can highlight
	{
		if (AUnit* CurrUnit = Cast<AUnit>(Hit.GetActor())) 
		{
			GameInstance->bIsUnitClicked = true;
			GameInstance->SelectedUnit = CurrUnit;

			if (CurrUnit->PlayerNumber == 1) 
			{
				AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
				if (!GameField) 
				{
					UE_LOG(LogTemp, Warning, TEXT("Couldn't cast the GameField"));
				}
				else
				{
					FVector2D XYPosition = CurrUnit->Position;
					TArray<bool> Visited;

					if ((CurrUnit->PawnType == EPawnType::SNIPER && !bSniperMoved) || (CurrUnit->PawnType == EPawnType::BRAWLER && !bBrawlerMoved))
					{
						Visited.Init(false, GameField->Size * GameField->Size);
						BFSMovementRange(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y), GameField->Size, CurrUnit->MovementRange, Visited, GameField);
						Visited.Init(false, GameField->Size * GameField->Size);
						BFSAttackRange(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y), GameField->Size, CurrUnit->AttackRange, Visited, GameField);
					}
					else if ((CurrUnit->PawnType == EPawnType::SNIPER && !bSniperAttacked) || (CurrUnit->PawnType == EPawnType::BRAWLER && !bBrawlerAttacked))
					{
						Visited.Init(false, GameField->Size * GameField->Size);
						BFSAttackRange(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y), GameField->Size, CurrUnit->AttackRange, Visited, GameField);
					}
					else
					{
						GameInstance->SelectedUnit = nullptr;
						GameInstance->bIsUnitClicked = false;
					}
						
				}
				
			}
		}
	}
}

void AHumanPlayer::ResetFlags()
{
	bBrawlerAttacked = false;
	bBrawlerMoved = false;
	bBrawlerPlaced = false;

	bSniperAttacked = false;
	bSniperMoved = false;
	bSniperPlaced = false;

	bIsMyTurn = false;
}



bool AHumanPlayer::CanAttack()
{
	bool condition = false;
	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	for (AUnit* Unit : MyUnits)
	{
		if (Unit)  // we need to see if at least one of your unit can attack
		{
			if ((Unit->PawnType == EPawnType::SNIPER && this->bSniperAttacked) || (Unit->PawnType == EPawnType::BRAWLER && this->bBrawlerAttacked))
			{
				continue;
			}
			else
			{
				TArray<bool> Visited;
				Visited.Init(false, GameField->Size * GameField->Size);
				BFSAttackRange(static_cast<int32>(Unit->Position.X), static_cast<int32>(Unit->Position.Y), GameField->Size, Unit->AttackRange, Visited, GameField);
				for (ATile* Tile : GameField->TileArray)
				{
					if (Tile->bIsRed)
					{
						condition = true;
						GameField->UnHighLight();
						break;
					}
				}
			}

			
		}
	}

	if (condition) 
	{
		UE_LOG(LogTemp, Warning, TEXT("CanAttack(): true"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CanAttack(): false"));
		GameField->UnHighLight();
	}
		

	return condition;
}