// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomPlayer.h"
#include "HumanPlayer.h"
#include "game_PlayerController.h"

// method to search and highlight all the tiles the unit can visit
void ARandomPlayer::BFSMovementRange(int32 startX, int32 startY, int32 size, int32 maxSteps, TArray<bool>& visited, AGameField* GF)
{
	TQueue<FIntPoint> queue;
	TQueue<int32> distanceQueue; 

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

		// if dist >= maxSteps, we cant explore anymore
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


// method to search and highlight all the units the unit can attack
void ARandomPlayer::BFSAttackRange(int32 startX, int32 startY, int32 size, int32 maxSteps, TArray<bool>& visited, AGameField* GF)
{
	TQueue<FIntPoint> queue;
	TQueue<int32> distanceQueue; 

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

		// if dist >= maxSteps, we cant explore anymore
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
					if (!GF->TileArray[newIndex]->bIsObstacle && GF->TileArray[newIndex]->PlayerOwner == 1)
						GF->TileArray[newIndex]->HighLight();

					queue.Enqueue(FIntPoint(nx, ny));
					distanceQueue.Enqueue(dist + 1);
				}
			}
		}
	}
}

// method to do highlight attack range and movement range and start a timer to let brawler move
void ARandomPlayer::HighlightAndMoveBrawler()
{

	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	AUnit* Brawler = nullptr;
	for (AUnit* Unit : MyUnits)
	{
		if (Unit->PawnType == EPawnType::BRAWLER)
		{
			Brawler = Unit;
			break;
		}
	}

	if (!Brawler)
	{
		UE_LOG(LogTemp, Warning, TEXT("Brawler not found"));
		return;
	}

	TArray<bool> Visited;
	Visited.Init(false, GameField->Size * GameField->Size);
	BFSMovementRange(static_cast<int32>(Brawler->Position.X), static_cast<int32>(Brawler->Position.Y), GameField->Size, Brawler->MovementRange, Visited, GameField);
	Visited.Init(false, GameField->Size * GameField->Size);
	BFSAttackRange(static_cast<int32>(Brawler->Position.X), static_cast<int32>(Brawler->Position.Y), GameField->Size, Brawler->AttackRange, Visited, GameField);

	GetWorldTimerManager().SetTimer(BrawlerMoveTimerHandle, this, &ARandomPlayer::MoveBrawler, 1.f, false);
	
}

// method to do highlight attack range and movement range and start a timer to let sniper move
void ARandomPlayer::HighlightAndMoveSniper()
{
	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	AUnit* Sniper = nullptr;
	for (AUnit* Unit : MyUnits)
	{
		if (Unit->PawnType == EPawnType::SNIPER)
		{
			Sniper = Unit;
			break;
		}
	}

	if (!Sniper)
	{
		UE_LOG(LogTemp, Warning, TEXT("Sniper not found"));
		return;
	}

	TArray<bool> Visited;
	Visited.Init(false, GameField->Size * GameField->Size);
	BFSMovementRange(static_cast<int32>(Sniper->Position.X), static_cast<int32>(Sniper->Position.Y), GameField->Size, Sniper->MovementRange, Visited, GameField);
	Visited.Init(false, GameField->Size * GameField->Size);
	BFSAttackRange(static_cast<int32>(Sniper->Position.X), static_cast<int32>(Sniper->Position.Y), GameField->Size, Sniper->AttackRange, Visited, GameField);

	GetWorldTimerManager().SetTimer(SniperMoveTimerHandle, this, &ARandomPlayer::MoveSniper, 1.f, false);
}

// method to handle ai movement for brawler
void ARandomPlayer::MoveBrawler()
{
	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));

	if (bIsMyTurn)
	{
		// he finds his Brawler
		AUnit* Brawler = nullptr;
		for (AUnit* Unit : MyUnits)
		{
			if (Unit->PawnType == EPawnType::BRAWLER)
			{
				Brawler = Unit;
				break;
			}
		}

		if (!Brawler)
		{
			UE_LOG(LogTemp, Warning, TEXT("Brawler not found"));
			return;
		}

		AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());

		if (!GameModality)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameModality not found"));
			return;
		}

		// now he search for the enemy sniper and brawler
		AHumanPlayer* HumanPlayer = Cast<AHumanPlayer>(GameModality->Players[0]);
		AUnit* EnemySniper = nullptr;
		AUnit* EnemyBrawler = nullptr;
		for (AUnit* Enemy : HumanPlayer->MyUnits)
		{
			if (Enemy)
			{
				if (Enemy->PawnType == EPawnType::SNIPER)
				{
					EnemySniper = Enemy;
				}
				else
				{
					EnemyBrawler = Enemy;
				}
			}
		}


		int32 RandomNumber = FMath::RandRange(0, 1);

		FVector2D XYPosition(-1, -1);
		TArray<ATile*> VisitableTiles;

		// if is smart he goes after a unit, else he goes to a random one
		if (bIsSmart)
		{
			int32 TileToBrawler = INT_MAX, TileToSniper = INT_MAX;

			RandomNumber = FMath::RandRange(0, 1);

			if (EnemyBrawler)
			{
				TileToBrawler = FMath::Abs(Brawler->Position.X - EnemyBrawler->Position.X) + FMath::Abs(Brawler->Position.Y - EnemyBrawler->Position.Y);
			}

			if (EnemySniper)
			{
				TileToSniper = FMath::Abs(Brawler->Position.X - EnemySniper->Position.X) + FMath::Abs(Brawler->Position.Y - EnemySniper->Position.Y);
			}


			FVector2D DestinationBrawler(-1, -1);
			FVector2D DestinationSniper(-1, -1);

			if(EnemyBrawler)
			{
				DestinationBrawler = FindAStarDestination(Brawler, EnemyBrawler);
			}
			if (EnemySniper)
			{
				DestinationSniper = FindAStarDestination(Brawler, EnemySniper);
			}
			
			
			// if he cant reach a unit he goes to another, if he cant either he goes to a random visitable tile
			if (!(DestinationBrawler.X == -1 && DestinationSniper.X == -1))
			{
				if (TileToBrawler < TileToSniper || DestinationSniper.X == -1)
				{
					XYPosition = DestinationBrawler;
				}
				else if (TileToBrawler > TileToSniper || DestinationBrawler.X == -1)
				{
					XYPosition = FindAStarDestination(Brawler, EnemySniper);
				}
				else if (TileToBrawler == TileToSniper)
				{

					if (RandomNumber == 0)
					{
						XYPosition = FindAStarDestination(Brawler, EnemySniper);
					}
					else
					{
						XYPosition = FindAStarDestination(Brawler, EnemyBrawler);
					}
				}
			}
			else
			{
				for (ATile* Tile : GameField->TileArray)
				{
					if (Tile->bIsGreen)
					{
						VisitableTiles.Add(Tile);
					}
				}
				if (VisitableTiles.Num() > 0)
				{
					int32 RandomIndex = FMath::RandRange(0, VisitableTiles.Num() - 1);
					XYPosition = GameField->GetXYPositionByRelativeLocation(VisitableTiles[RandomIndex]->GetActorLocation());
				}
			}
			
		}
		else
		{
			for (ATile* Tile : GameField->TileArray)
			{
				if (Tile->bIsGreen)
				{
					VisitableTiles.Add(Tile);
				}
			}
			if (VisitableTiles.Num() > 0)
			{
				int32 RandomIndex = FMath::RandRange(0, VisitableTiles.Num() - 1);
				XYPosition = GameField->GetXYPositionByRelativeLocation(VisitableTiles[RandomIndex]->GetActorLocation());
			}
		}

		if (!(XYPosition.X == -1 && XYPosition.Y == -1))
		{
			FString UnitID = (Brawler->PawnType == EPawnType::BRAWLER) ? TEXT("B") : TEXT("S");
			FString PlayerID = TEXT("IA");
			FString OriginCell = GetCellString(Brawler->Position);
			FString DestinationCell = GetCellString(XYPosition);

			Agame_PlayerController* PlayerC = Cast<Agame_PlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
			PlayerC->HUD->AddTextToScrollBox(FString::Printf(TEXT("%s: %s %s -> %s"), *PlayerID, *UnitID, *OriginCell, *DestinationCell));

			GameField->TileArray[static_cast<int32>(Brawler->Position.X) * GameField->Size + static_cast<int32>(Brawler->Position.Y)]->SetTileStatus(-1, ETileStatus::EMPTY);
			Brawler->FindPathAndMove(GameField->GetRelativeLocationByXYPosition(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y)), GameField);
			UE_LOG(LogTemp, Warning, TEXT("Going to x:%i y:%i"), static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y));
			GameField->TileArray[static_cast<int32>(XYPosition.X) * GameField->Size + static_cast<int32>(XYPosition.Y)]->SetTileStatus(2, ETileStatus::OCCUPIED);
		}
		bBrawlerMoved = true;
	}
	
	GameField->UnHighLight();
}

// method to handle ai movement for sniper
void ARandomPlayer::MoveSniper()
{
	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	if (bIsMyTurn)
	{
		AUnit* Sniper = nullptr;
		for (AUnit* Unit : MyUnits)
		{
			if (Unit->PawnType == EPawnType::SNIPER)
			{
				Sniper = Unit;
				break;
			}
		}

		if (!Sniper)
		{
			UE_LOG(LogTemp, Warning, TEXT("Sniper not found"));
			return;
		}

		AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());

		if (!GameModality)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameModality not found"));
			return;
		}

		AHumanPlayer* HumanPlayer = Cast<AHumanPlayer>(GameModality->Players[0]);
		AUnit* EnemySniper = nullptr;
		AUnit* EnemyBrawler = nullptr;
		for (AUnit* Enemy : HumanPlayer->MyUnits)
		{
			if (Enemy)
			{
				if (Enemy->PawnType == EPawnType::SNIPER)
				{
					EnemySniper = Enemy;
				}
				else
				{
					EnemyBrawler = Enemy;
				}
			}
		}

		int32 RandomNumber = FMath::RandRange(0, 1);

		FVector2D XYPosition(-1, -1);
		TArray<ATile*> VisitableTiles;

		// if is smart he goes after a unit, else he goes to a random one
		if (bIsSmart)
		{
			int32 TileToBrawler = INT_MAX, TileToSniper = INT_MAX;

			RandomNumber = FMath::RandRange(0, 1);

			if (EnemyBrawler)
			{
				TileToBrawler = FMath::Abs(Sniper->Position.X - EnemyBrawler->Position.X) + FMath::Abs(Sniper->Position.Y - EnemyBrawler->Position.Y);
			}

			if (EnemySniper)
			{
				TileToSniper = FMath::Abs(Sniper->Position.X - EnemySniper->Position.X) + FMath::Abs(Sniper->Position.Y - EnemySniper->Position.Y);
			}


			FVector2D DestinationBrawler(-1, -1);
			FVector2D DestinationSniper(-1, -1);

			if (EnemyBrawler)
			{
				DestinationBrawler = FindAStarDestination(Sniper, EnemyBrawler);
			}
			if (EnemySniper)
			{
				DestinationSniper = FindAStarDestination(Sniper, EnemySniper);
			}

			// if he cant reach a unit he goes to another, if he cant either he goes to a random visitable tile
			if (!(DestinationBrawler.X == -1 && DestinationSniper.X == -1))
			{
				if (TileToBrawler < TileToSniper || DestinationSniper.X == -1)
				{
					XYPosition = DestinationBrawler;
				}
				else if (TileToBrawler > TileToSniper || DestinationBrawler.X == -1)
				{
					XYPosition = FindAStarDestination(Sniper, EnemySniper);
				}
				else if (TileToBrawler == TileToSniper)
				{

					if (RandomNumber == 0)
					{
						XYPosition = FindAStarDestination(Sniper, EnemySniper);
					}
					else
					{
						XYPosition = FindAStarDestination(Sniper, EnemyBrawler);
					}
				}
			}
		}
		else
		{
			for (ATile* Tile : GameField->TileArray)
			{
				if (Tile->bIsGreen)
				{
					VisitableTiles.Add(Tile);
				}
			}

			int32 RandomIndex = FMath::RandRange(0, VisitableTiles.Num() - 1);

			XYPosition = GameField->GetXYPositionByRelativeLocation(VisitableTiles[RandomIndex]->GetActorLocation());

		}


		if (!(XYPosition.X == -1 && XYPosition.Y == -1))
		{
			FString UnitID = (Sniper->PawnType == EPawnType::BRAWLER) ? TEXT("B") : TEXT("S");
			FString PlayerID = TEXT("IA");
			FString OriginCell = GetCellString(Sniper->Position);
			FString DestinationCell = GetCellString(XYPosition);

			Agame_PlayerController* PlayerC = Cast<Agame_PlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
			PlayerC->HUD->AddTextToScrollBox(FString::Printf(TEXT("%s: %s %s -> %s"), *PlayerID, *UnitID, *OriginCell, *DestinationCell));

			GameField->TileArray[static_cast<int32>(Sniper->Position.X) * GameField->Size + static_cast<int32>(Sniper->Position.Y)]->SetTileStatus(-1, ETileStatus::EMPTY);
			Sniper->FindPathAndMove(GameField->GetRelativeLocationByXYPosition(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y)), GameField);
			GameField->TileArray[static_cast<int32>(XYPosition.X) * GameField->Size + static_cast<int32>(XYPosition.Y)]->SetTileStatus(2, ETileStatus::OCCUPIED);
		}

		bSniperMoved = true;
	}
	

	GameField->UnHighLight();
	
}

// method to highlight the enemy units on range
void ARandomPlayer::HighlightAndAttackBrawler()
{
	if (bIsMyTurn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Brawler attacking"));
		AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
		AUnit* Brawler = nullptr;
		for (AUnit* Unit : MyUnits)
		{
			if (Unit->PawnType == EPawnType::BRAWLER)
			{
				Brawler = Unit;
				break;
			}
		}

		if (!Brawler)
		{
			UE_LOG(LogTemp, Warning, TEXT("Brawler not found"));
			return;
		}

		TArray<bool> Visited;
		Visited.Init(false, GameField->Size * GameField->Size);
		BFSAttackRange(static_cast<int32>(Brawler->Position.X), static_cast<int32>(Brawler->Position.Y), GameField->Size, Brawler->AttackRange, Visited, GameField);

		GetWorldTimerManager().SetTimer(BrawlerAttackTimerHandle, this, &ARandomPlayer::AttackBrawler, 1.f, false);
	}
}

// method to highlight the enemy units on range
void ARandomPlayer::HighlightAndAttackSniper()
{
	if (bIsMyTurn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Sniper attacking"));
		AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
		AUnit* Sniper = nullptr;
		for (AUnit* Unit : MyUnits)
		{
			if (Unit->PawnType == EPawnType::SNIPER)
			{
				Sniper = Unit;
				break;
			}
		}

		if (!Sniper)
		{
			UE_LOG(LogTemp, Warning, TEXT("Sniper not found"));
			return;
		}

		TArray<bool> Visited;
		Visited.Init(false, GameField->Size * GameField->Size);
		BFSAttackRange(static_cast<int32>(Sniper->Position.X), static_cast<int32>(Sniper->Position.Y), GameField->Size, Sniper->AttackRange, Visited, GameField);

		GetWorldTimerManager().SetTimer(SniperAttackTimerHandle, this, &ARandomPlayer::AttackSniper, 1.f, false);
	}
}

// method to let brawler attack, if he has more than one enemy unit on range he shots a random one
void ARandomPlayer::AttackBrawler()
{
	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	if (bIsMyTurn)
	{
		AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
		AUnit* Brawler = nullptr;
		for (AUnit* Unit : MyUnits)
		{
			if (Unit->PawnType == EPawnType::BRAWLER)
			{
				Brawler = Unit;
				break;
			}
		}

		if (!Brawler)
		{
			UE_LOG(LogTemp, Warning, TEXT("Brawler not found"));
			return;
		}

		TArray<AUnit*> EnemyAtRange;
		AHumanPlayer* HumanPlayer = Cast<AHumanPlayer>(Cast<AGameModality>(GetWorld()->GetAuthGameMode())->Players[0]);
		for (ATile* Tile : GameField->TileArray)
		{
			if (Tile->bIsRed)
			{
				for (AUnit* Unit : HumanPlayer->MyUnits)
				{
					if (Unit->Position == GameField->GetXYPositionByRelativeLocation(Tile->GetActorLocation()))
					{
						EnemyAtRange.Add(Unit);
					}
				}
			}
		}

		if (EnemyAtRange.Num() == 1)
		{
			Brawler->Attack(EnemyAtRange[0]);
		}
		else if (EnemyAtRange.Num() == 2)
		{
			int32 RandomNumber = FMath::RandRange(0, 1);
			Brawler->Attack(EnemyAtRange[RandomNumber]);
		}


		if (GameModality)
		{
			if (GameModality->CheckWin())
				return;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Couldn't find GameMode"));
		}
	}

	GameField->UnHighLight();
}

// method to let sniper attack, if he has more than one enemy unit on range he shots a random one
void ARandomPlayer::AttackSniper()
{

	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	if (bIsMyTurn)
	{
		AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
		AUnit* Sniper = nullptr;
		for (AUnit* Unit : MyUnits)
		{
			if (Unit->PawnType == EPawnType::SNIPER)
			{
				Sniper = Unit;
				break;
			}
		}

		if (!Sniper)
		{
			UE_LOG(LogTemp, Warning, TEXT("Sniper not found"));
			return;
		}

		TArray<AUnit*> EnemyAtRange;
		AHumanPlayer* HumanPlayer = Cast<AHumanPlayer>(Cast<AGameModality>(GetWorld()->GetAuthGameMode())->Players[0]);
		for (ATile* Tile : GameField->TileArray)
		{
			if (Tile->bIsRed)
			{
				for (AUnit* Unit : HumanPlayer->MyUnits)
				{
					if (Unit->Position == GameField->GetXYPositionByRelativeLocation(Tile->GetActorLocation()))
					{
						EnemyAtRange.Add(Unit);
					}
				}
			}
		}

		if (EnemyAtRange.Num() == 1)
		{
			Sniper->Attack(EnemyAtRange[0]);
		}
		else if (EnemyAtRange.Num() == 2)
		{
			int32 RandomNumber = FMath::RandRange(0, 1);
			Sniper->Attack(EnemyAtRange[RandomNumber]);
		}

		if (GameModality)
		{
			if (GameModality->CheckWin())
				return;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Couldn't find GameMode"));
		}

	}

	GameField->UnHighLight();
}


// Sets default values
ARandomPlayer::ARandomPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GameInstance = Cast<Ugame_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	bBrawlerPlaced = false;
	bSniperPlaced = false;
	bBrawlerAttacked = false;
	bSniperAttacked = false;	
	bBrawlerMoved = false;
	bSniperMoved = false;
	bIsSmart = false;
}

// Called when the game starts or when spawned
void ARandomPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARandomPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ARandomPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}



void ARandomPlayer::OnTurn()
{
	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	if (GameField)
	{
		GameField->UnHighLight();
	}
	Agame_PlayerController* PlayerC = Cast<Agame_PlayerController>(UGameplayStatics::GetActorOfClass(GetWorld(), Agame_PlayerController::StaticClass()));
	if (PlayerC && PlayerC->HUD)
		PlayerC->HUD->SetTurnText(2);

	bBrawlerAttacked = false;
	bSniperAttacked = false;
	bBrawlerMoved = false;
	bSniperMoved = false;
	bIsMyTurn = true;
	
	// if there is a unit to place he does
	if (!(bSniperPlaced && bBrawlerPlaced))
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle1, [&]()
			{
				if (bIsMyTurn)
				{
					AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
					AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
					int32 RandomNumber, Rand;
					Rand = -1;
					do {
						RandomNumber = FMath::RandRange(0, GameField->Size * GameField->Size - 1);
					} while (GameField->TileArray[RandomNumber]->GetTileStatus() == ETileStatus::OCCUPIED);

					UE_LOG(LogTemp, Warning, TEXT("Random index: %i"), RandomNumber);

					// if no unit is placed he place a random one
					if (!(bSniperPlaced || bBrawlerPlaced))
					{
						Rand = FMath::RandRange(0, 1);
					}

					if (bSniperPlaced || Rand == 0)
					{

						GameField->TileArray[RandomNumber]->SetTileStatus(2, ETileStatus::OCCUPIED);

						int32 X = RandomNumber / GameField->Size;
						int32 Y = RandomNumber % GameField->Size;

						FVector Position = GameField->GetRelativeLocationByXYPosition(X, Y);
						Position.Z = 1;
						AUnit* Unit = GameModality->SpawnCellUnit(2, Position, EPawnType::BRAWLER);
						bBrawlerPlaced = true;
						if (Unit)
						{
							MyUnits.Add(Unit);
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("Null Unit"));
						}


					}
					else if (bBrawlerPlaced || Rand == 1)
					{
						int32 X = RandomNumber / GameField->Size;
						int32 Y = RandomNumber % GameField->Size;

						FVector Position = GameField->GetRelativeLocationByXYPosition(X, Y);
						Position.Z = 1;
						AUnit* Unit = GameModality->SpawnCellUnit(2, Position, EPawnType::SNIPER);
						bSniperPlaced = true;
						if (Unit)
						{
							MyUnits.Add(Unit);
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("Null Unit"));
						}


					}

					// every time he places a unit he updates the Unit HP TEXT
					Agame_PlayerController* PlayerC = Cast<Agame_PlayerController>(UGameplayStatics::GetActorOfClass(GetWorld(), Agame_PlayerController::StaticClass()));
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

		}, 1, false);
	
	}
	else
	{
		int32 RandomNumber = FMath::RandRange(0, 1);

		AUnit* Brawler = nullptr;
		AUnit* Sniper = nullptr;
		waitTime = 1.f;

		for (AUnit* Unit : MyUnits)
		{
			if (Unit->PawnType == EPawnType::BRAWLER)
			{
				Brawler = Unit;
			}
			else if (Unit->PawnType == EPawnType::SNIPER)
			{
				Sniper = Unit;
			}
		}

		// if he can attack he attacks (there is no difference between attacking firs with one or another)
		if (Brawler && Brawler->CanAttack())
		{
			GetWorld()->GetTimerManager().SetTimer(BrawlerAttackTimerHandle, this, &ARandomPlayer::HighlightAndAttackBrawler, waitTime, false);
			waitTime += 2;
			bBrawlerAttacked = true;
		}

		if (Sniper && Sniper->CanAttack())
		{
			GetWorld()->GetTimerManager().SetTimer(SniperAttackTimerHandle, this, &ARandomPlayer::HighlightAndAttackSniper, waitTime, false);
			waitTime += 2;
			bSniperAttacked = true;
		}

		// then if they didn't already attacked they try to move 
		if (RandomNumber == 0)
		{
			if (Brawler && !bBrawlerAttacked) 
			{
				GetWorld()->GetTimerManager().SetTimer(BrawlerMoveTimerHandle, this, &ARandomPlayer::HighlightAndMoveBrawler, waitTime, false);
				waitTime += 2.5;
			}
			if (Sniper && !bSniperAttacked)
			{
				GetWorld()->GetTimerManager().SetTimer(SniperMoveTimerHandle, this, &ARandomPlayer::HighlightAndMoveSniper, waitTime, false);
				waitTime += 2.5;
			}
		}
		else
		{
			if (Sniper && !bSniperAttacked)
			{
				GetWorld()->GetTimerManager().SetTimer(SniperMoveTimerHandle, this, &ARandomPlayer::HighlightAndMoveSniper, waitTime, false);
				waitTime += 2.5;
			}
			if (Brawler && !bBrawlerAttacked)
			{
				GetWorld()->GetTimerManager().SetTimer(BrawlerMoveTimerHandle, this, &ARandomPlayer::HighlightAndMoveBrawler, waitTime, false);
				waitTime += 2.5;
			}
		}
		
		// if they only moved they didn't attack they try to attack 
		GetWorld()->GetTimerManager().SetTimer(CanAttackTimerHandle, [&]()
			{
				int32 RandomNumber = FMath::RandRange(0, 1);
				float wait = 0.1f;
				AUnit* Brawler = nullptr;
				AUnit* Sniper = nullptr;
				for (AUnit* Unit : MyUnits)
				{
					if (Unit->PawnType == EPawnType::BRAWLER)
					{
						Brawler = Unit;
					}
					else if (Unit->PawnType == EPawnType::SNIPER)
					{
						Sniper = Unit;
					}
				}

				if (RandomNumber == 0)
				{
					if (Brawler && !bBrawlerAttacked && Brawler->CanAttack())
					{
						GetWorld()->GetTimerManager().SetTimer(BrawlerAttackTimerHandle, this, &ARandomPlayer::HighlightAndAttackBrawler, wait, false);
						bool IsActive = GetWorld()->GetTimerManager().IsTimerActive(BrawlerAttackTimerHandle);
						UE_LOG(LogTemp, Warning, TEXT("BrawlerAttackTimerHandle active after SetTimer: %s"), IsActive ? TEXT("YES") : TEXT("NO"));
						wait += 2;
					}
					if (Sniper && !bSniperAttacked && Sniper->CanAttack())
					{
						GetWorld()->GetTimerManager().SetTimer(SniperAttackTimerHandle, this, &ARandomPlayer::HighlightAndAttackSniper, wait, false);
						bool IsActive = GetWorld()->GetTimerManager().IsTimerActive(SniperAttackTimerHandle);
						UE_LOG(LogTemp, Warning, TEXT("SniperAttackTimerHandle active after SetTimer: %s"), IsActive ? TEXT("YES") : TEXT("NO"));
						wait += 2;
					}
				}
				else
				{
					if (Sniper && !bSniperAttacked && Sniper->CanAttack())
					{
						GetWorld()->GetTimerManager().SetTimer(SniperAttackTimerHandle, this, &ARandomPlayer::HighlightAndAttackSniper, wait, false);
						wait += 2.5;
						bool IsActive = GetWorld()->GetTimerManager().IsTimerActive(SniperAttackTimerHandle);
						UE_LOG(LogTemp, Warning, TEXT("SniperAttackTimerHandle active after SetTimer: %s"), IsActive ? TEXT("YES") : TEXT("NO"));
					}
					if (Brawler && !bBrawlerAttacked && Brawler->CanAttack())
					{
						GetWorld()->GetTimerManager().SetTimer(BrawlerAttackTimerHandle, this, &ARandomPlayer::HighlightAndAttackBrawler, wait, false);
						wait += 2.5;
						bool IsActive = GetWorld()->GetTimerManager().IsTimerActive(BrawlerAttackTimerHandle);
						UE_LOG(LogTemp, Warning, TEXT("BrawlerAttackTimerHandle active after SetTimer: %s"), IsActive ? TEXT("YES") : TEXT("NO"));
					}
				}


				UE_LOG(LogTemp, Warning, TEXT("Finita lambda function"));
			}, waitTime, false);

		waitTime += 2;
		AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
		if (GameModality)
		{
			GetWorld()->GetTimerManager().SetTimer(NextTurnTimerHandle, GameModality, &AGameModality::TurnNextPlayer, waitTime, false);
			bool IsActive = GetWorld()->GetTimerManager().IsTimerActive(NextTurnTimerHandle);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GameModality not found"));
		}
	}
}

void ARandomPlayer::ResetFlags()
{
	bBrawlerAttacked = false;
	bBrawlerMoved = false;
	bBrawlerPlaced = false;

	bSniperAttacked = false;
	bSniperMoved = false;
	bSniperPlaced = false;
}


// method to just count steps from a start to a destination
/*int32 ARandomPlayer::CountStepsBFS(const FVector2D& Start, const FVector2D& Goal, AGameField* GF)
{
	// Converte le coordinate in indice (assumendo che Start e Goal siano in coordinate griglia intere)
	int32 StartIndex = static_cast<int32>(Start.X) * GF->Size + static_cast<int32>(Start.Y);
	int32 GoalIndex = static_cast<int32>(Goal.X) * GF->Size + static_cast<int32>(Goal.Y);


	// Se partenza e arrivo coincidono, ritorna 0
	if (StartIndex == GoalIndex)
	{
		UE_LOG(LogTemp, Warning, TEXT("You're on your destination"));
		return 0;
	}

	// Definisci un array di indici target.
	// Se la tile di destinazione non è vuota, il target sarà direttamente quella tile;
	// altrimenti, il target sarà una delle tile adiacenti.
	TArray<int32> TargetIndices;
	if (GF->TileArray[GoalIndex]->GetTileStatus() == ETileStatus::OCCUPIED)
	{
		// La tile di destinazione è vuota: usa le tile adiacenti
		int32 GoalX = static_cast<int32>(Goal.X);
		int32 GoalY = static_cast<int32>(Goal.Y);
		if (GoalX > 0 && GF->TileArray[(GoalX - 1) * GF->Size + GoalY]->GetTileStatus() == ETileStatus::EMPTY)
		{
			TargetIndices.Add((GoalX - 1) * GF->Size + GoalY);
		}

		if (GoalX < GF->Size - 1 && GF->TileArray[(GoalX + 1) * GF->Size + GoalY]->GetTileStatus() == ETileStatus::EMPTY)
		{
			TargetIndices.Add((GoalX + 1) * GF->Size + GoalY);
		}

		if (GoalY > 0 && GF->TileArray[GoalX * GF->Size + (GoalY - 1)]->GetTileStatus() == ETileStatus::EMPTY)
		{
			TargetIndices.Add(GoalX * GF->Size + (GoalY - 1));
		}
		if (GoalY < GF->Size - 1 && GF->TileArray[GoalX * GF->Size + (GoalY + 1)]->GetTileStatus() == ETileStatus::EMPTY)
		{
			TargetIndices.Add(GoalX * GF->Size + (GoalY + 1));
		}

		if (TargetIndices.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("No valid target tiles found!"));
			return INT_MAX;
		}

	}
	else
	{
		// La tile di destinazione non è vuota: il target è la tile stessa
		TargetIndices.Add(GoalIndex);
	}


	// Inizializza l'array dei passi
	TArray<int32> Steps;
	Steps.Init(-1, GF->Size * GF->Size);
	Steps[StartIndex] = 0;

	// Coda per la BFS
	TQueue<int32> Queue;
	Queue.Enqueue(StartIndex);

	while (!Queue.IsEmpty())
	{
		int32 Current;
		Queue.Dequeue(Current);

		int32 x = Current / GF->Size;
		int32 y = Current % GF->Size;

		static const int32 DirX[4] = { 1, -1, 0, 0 };
		static const int32 DirY[4] = { 0, 0, 1, -1 };

		for (int i = 0; i < 4; i++)
		{
			int32 nx = x + DirX[i];
			int32 ny = y + DirY[i];

			if (nx >= 0 && nx < GF->Size && ny >= 0 && ny < GF->Size)
			{
				int32 NextIndex = nx * GF->Size + ny;

				// Controlla che la tile sia libera e non ancora visitata
				if (GF->TileArray[NextIndex]->GetTileStatus() == ETileStatus::EMPTY && Steps[NextIndex] == -1)
				{
					Steps[NextIndex] = Steps[Current] + 1;

					// Se il NextIndex è uno degli obiettivi, ritorna il numero di passi
					if (TargetIndices.Contains(NextIndex))
					{
						return Steps[NextIndex];
					}

					Queue.Enqueue(NextIndex);
				}
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("No path found"));
	return INT_MAX;
}*/

int32 ARandomPlayer::AStarSearch(const FVector2D& Start, const FVector2D& Goal, TArray<FVector2D>& Path)
{
	// Get the game field object
	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	if (!GameField)
	{
		// Return -1 if no game field is found
		return -1;
	}

	// Node structure for A* algorithm
	struct FNode
	{
		FVector2D Position;  // Position of the node
		int32 GCost;         // Distance from the start node
		int32 HCost;         // Estimated distance to the goal (heuristic)
		int32 FCost() const { return GCost + HCost; } // Total cost (G + H)
		FNode* Parent;       // Parent node for path reconstruction

		// Constructor for initializing node with position, G, H costs, and parent
		FNode(FVector2D Pos, int32 G, int32 H, FNode* P = nullptr)
			: Position(Pos), GCost(G), HCost(H), Parent(P) {
		}
	};

	// OpenSet stores nodes to be explored, ClosedSet stores already explored nodes
	TMap<FVector2D, FNode*> OpenSet;
	TMap<FVector2D, FNode*> ClosedSet;
	TArray<FNode*> AllocatedNodes; // To clean up memory at the end

	// Initialize the start node
	FNode* StartNode = new FNode(Start, 0, FMath::Abs(Start.X - Goal.X) + FMath::Abs(Start.Y - Goal.Y));
	OpenSet.Add(Start, StartNode);
	AllocatedNodes.Add(StartNode);

	// Main loop of A* algorithm
	while (OpenSet.Num() > 0)
	{
		// Find the node with the lowest F cost
		FNode* CurrentNode = nullptr;
		for (auto& Pair : OpenSet)
		{
			if (!CurrentNode || Pair.Value->FCost() < CurrentNode->FCost())
			{
				CurrentNode = Pair.Value;
			}
		}

		// If no valid node found, break out
		if (!CurrentNode)
			break;

		FVector2D CurrentPos = CurrentNode->Position;
		OpenSet.Remove(CurrentPos);
		ClosedSet.Add(CurrentPos, CurrentNode);

		// If the goal is reached, reconstruct the path
		if (CurrentPos == Goal)
		{
			// Reconstruct the path by following the parent nodes
			while (CurrentNode)
			{
				Path.Insert(CurrentNode->Position, 0);
				CurrentNode = CurrentNode->Parent;
			}

			// Return the cost of the path (length of the path minus 1)
			int32 Cost = Path.Num() - 1;

			// Clean up allocated nodes
			for (FNode* Node : AllocatedNodes)
			{
				delete Node;
			}

			return Cost;
		}

		// Directions: right, left, down, up
		static const int32 DirX[4] = { 1, -1, 0, 0 };
		static const int32 DirY[4] = { 0, 0, 1, -1 };
		// Explore each neighbor of the current node
		for (int i = 0; i < 4; i++)
		{
			FVector2D NeighborPos = FVector2D(CurrentPos.X + DirX[i], CurrentPos.Y + DirY[i]);

			// Skip neighbors that are out of bounds
			if (NeighborPos.X < 0 || NeighborPos.X >= GameField->Size || NeighborPos.Y < 0 || NeighborPos.Y >= GameField->Size)
				continue;

			// Skip neighbors that are not empty (occupied tiles)
			int32 NeighborIndex = static_cast<int32>(NeighborPos.X) * GameField->Size + static_cast<int32>(NeighborPos.Y);
			if (GameField->TileArray[NeighborIndex]->GetTileStatus() != ETileStatus::EMPTY)
				continue;

			// Skip neighbors that have already been visited
			if (ClosedSet.Contains(NeighborPos))
				continue;

			// Calculate the new G and H costs for the neighbor node
			int32 NewGCost = CurrentNode->GCost + 1; // Movement cost is 1 for horizontal/vertical moves
			int32 NewHCost = FMath::Abs(NeighborPos.X - Goal.X) + FMath::Abs(NeighborPos.Y - Goal.Y);

			// If the neighbor node is not in the open set or has a better G cost, add it to the open set
			if (!OpenSet.Contains(NeighborPos) || NewGCost < OpenSet[NeighborPos]->GCost)
			{
				FNode* NeighborNode = new FNode(NeighborPos, NewGCost, NewHCost, CurrentNode);
				OpenSet.Add(NeighborPos, NeighborNode);
				AllocatedNodes.Add(NeighborNode);
			}
		}
	}

	// Clean up allocated nodes if no path is found
	for (FNode* Node : AllocatedNodes)
	{
		delete Node;
	}

	// Return -1 if no path was found
	return -1;
}

FVector2D ARandomPlayer::FindAStarDestination(AUnit* MovingUnit, AUnit* TargetUnit)
{
	// Get the game field object
	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	if (!MovingUnit || !TargetUnit || !GameField)
	{
		// Log an error if the parameters are invalid and return the current position of the moving unit
		UE_LOG(LogTemp, Error, TEXT("FindAStarDestination: Invalid parameters"));
		return MovingUnit ? MovingUnit->Position : FVector2D(-1, -1);
	}

	// Store the starting position and target position
	FVector2D Start = MovingUnit->Position;
	FVector2D Target = TargetUnit->Position;

	// If the unit is already adjacent to the target, no need to move
	if (FMath::Abs(Start.X - Target.X) + FMath::Abs(Start.Y - Target.Y) == 1)
	{
		return Start;
	}

	// Find the adjacent cells to the target that are free
	TArray<FVector2D> TargetTiles;
	int32 TargetX = static_cast<int32>(Target.X);
	int32 TargetY = static_cast<int32>(Target.Y);
	static const int32 DirX[4] = { 1, -1, 0, 0 };
	static const int32 DirY[4] = { 0, 0, 1, -1 };

	// Check the 4 adjacent directions (right, left, down, up)
	for (int i = 0; i < 4; i++)
	{
		int32 nx = TargetX + DirX[i];
		int32 ny = TargetY + DirY[i];

		// If the neighbor is within grid bounds
		if (nx >= 0 && nx < GameField->Size && ny >= 0 && ny < GameField->Size)
		{
			int32 TileIndex = nx * GameField->Size + ny;
			// If the tile is empty, add it to the list of possible target tiles
			if (GameField->TileArray[TileIndex]->GetTileStatus() == ETileStatus::EMPTY)
			{
				TargetTiles.Add(FVector2D(nx, ny));
			}
		}
	}

	// If no free cells are found around the target, stop at the current position
	if (TargetTiles.Num() == 0)
	{
		TargetTiles.Add(Target);
	}

	// Perform A* to find the best path from Start to one of the target tiles
	int32 BestCost = INT_MAX;
	TArray<FVector2D> BestPath;

	// Evaluate paths to each possible target tile
	for (const FVector2D& GoalTile : TargetTiles)
	{
		TArray<FVector2D> Path;
		int32 Cost = AStarSearch(Start, GoalTile, Path);
		// Update the best path if a better one is found
		if (Cost >= 0 && Cost < BestCost)
		{
			BestCost = Cost;
			BestPath = Path;
		}
	}

	// If no valid path was found, return the current position
	if (BestPath.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FindAStarDestination: No valid path found"));
		return FVector2D(-1, -1);
	}

	// If the best path length is less than or equal to the movement range, return the last position in the path
	if (BestPath.Num() - 1 <= MovingUnit->MovementRange)
	{
		return BestPath.Last();
	}
	else
	{
		// Otherwise, return the reachable tile after moving the max number of steps
		return BestPath[MovingUnit->MovementRange];
	}
}


FString ARandomPlayer::GetCellString(const FVector2D& CellCoord)
{
	// Convert the column to a letter: 0 -> A, 1 -> B, ...
	int32 Column = static_cast<int32>(CellCoord.X);
	TCHAR ColumnLetter = 'A' + Column;

	// Convert the row to a number (add 1 for 1-based indexing)
	int32 Row = static_cast<int32>(CellCoord.Y) + 1;

	// Combine the letter and number into a string
	return FString::Printf(TEXT("%c%d"), ColumnLetter, Row);
}

