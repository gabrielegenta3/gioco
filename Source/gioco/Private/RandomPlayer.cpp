// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomPlayer.h"
#include "HumanPlayer.h"
#include "game_PlayerController.h"


void ARandomPlayer::BFSMovementRange(int32 startX, int32 startY, int32 size, int32 maxSteps, TArray<bool>& visited, AGameField* GF)
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

		// Se dist >= maxSteps, non esploriamo più
		if (dist >= maxSteps)
			continue;

		// 4 direzioni
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
					GF->TileArray[newIndex]->LightUp();

					queue.Enqueue(FIntPoint(nx, ny));
					distanceQueue.Enqueue(dist + 1);
				}
			}
		}
	}
}


// BFS to find attackable units
void ARandomPlayer::BFSAttackRange(int32 startX, int32 startY, int32 size, int32 maxSteps, TArray<bool>& visited, AGameField* GF)
{
	TQueue<FIntPoint> queue;
	TQueue<int32> distanceQueue; // per salvare i passi correnti

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

		// Se dist >= maxSteps, non esploriamo più
		if (dist >= maxSteps)
			continue;

		// 4 direzioni
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
						GF->TileArray[newIndex]->LightUp();

					queue.Enqueue(FIntPoint(nx, ny));
					distanceQueue.Enqueue(dist + 1);
				}
			}
		}
	}
}

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

void ARandomPlayer::MoveBrawler()
{
	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));

	if (IsMyTurn)
	{
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

		if (bIsSmart)
		{
			FVector2D DestinationToBrawler;
			FVector2D DestinationToSniper;
			int32 TileToBrawler = INT_MAX, TileToSniper = INT_MAX;

			RandomNumber = FMath::RandRange(0, 1);

			if (EnemyBrawler)
			{
				DestinationToBrawler = FindAStarDestination(Brawler, EnemyBrawler);
				TileToSniper = FMath::Abs(Brawler->Position.X - EnemyBrawler->Position.X) + FMath::Abs(Brawler->Position.Y - EnemyBrawler->Position.Y);
			}

			if (EnemySniper)
			{
				DestinationToSniper = FindAStarDestination(Brawler, EnemySniper);
				TileToSniper = FMath::Abs(Brawler->Position.X - EnemySniper->Position.X) + FMath::Abs(Brawler->Position.Y - EnemySniper->Position.Y);
			}
			 
			if (TileToBrawler < TileToSniper)
			{
				XYPosition = DestinationToSniper;
			}
			else if (TileToBrawler > TileToSniper)
			{
				XYPosition = DestinationToBrawler;
			}
			else if (TileToBrawler == TileToSniper)
			{
				if (RandomNumber == 0)
				{
					XYPosition = DestinationToSniper;
				}
				else if (RandomNumber == 1)
				{
					XYPosition = DestinationToBrawler;
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
		BrawlerMoved = true;
	}
	
	GameField->UnHighLight();
}

void ARandomPlayer::MoveSniper()
{
	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	if (IsMyTurn)
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

		if (bIsSmart)
		{
			FVector2D DestinationToBrawler;
			FVector2D DestinationToSniper;
			int32 TileToBrawler = INT_MAX, TileToSniper = INT_MAX;

			RandomNumber = FMath::RandRange(0, 1);

			if (EnemyBrawler)
			{
				DestinationToBrawler = FindAStarDestination(Sniper, EnemyBrawler);
				TileToSniper = FMath::Abs(Sniper->Position.X - EnemyBrawler->Position.X) + FMath::Abs(Sniper->Position.Y - EnemyBrawler->Position.Y);
				// TileToBrawler = CountStepsBFS(DestinationToBrawler, EnemyBrawler->Position, GameField);
			}

			if (EnemySniper)
			{
				DestinationToSniper = FindAStarDestination(Sniper, EnemySniper);
				TileToSniper = FMath::Abs(Sniper->Position.X - EnemySniper->Position.X) + FMath::Abs(Sniper->Position.Y - EnemySniper->Position.Y);
				//TileToSniper = CountStepsBFS(DestinationToBrawler, EnemySniper->Position, GameField);
			}


			if (TileToBrawler > TileToSniper)
			{
				XYPosition = DestinationToSniper;
			}
			else if (TileToBrawler < TileToSniper)
			{
				XYPosition = DestinationToBrawler;
			}
			else if (TileToBrawler == TileToSniper)
			{
				if (RandomNumber == 0)
				{
					XYPosition = DestinationToSniper;
				}
				else if (RandomNumber == 1)
				{
					XYPosition = DestinationToBrawler;
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

		SniperMoved = true;
	}
	

	GameField->UnHighLight();
	
}

void ARandomPlayer::HighlightAndAttackBrawler()
{
	if (IsMyTurn)
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

void ARandomPlayer::HighlightAndAttackSniper()
{
	if (IsMyTurn)
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

void ARandomPlayer::AttackBrawler()
{
	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	if (IsMyTurn)
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


		if (GameModality && GameModality->CheckWin())
		{
			return;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Couldn't find GameMode"));
		}
	}

	GameField->UnHighLight();
}

void ARandomPlayer::AttackSniper()
{

	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	if (IsMyTurn)
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
	BrawlerPlaced = false;
	SniperPlaced = false;
	BrawlerAttacked = false;
	SniperAttacked = false;	
	BrawlerMoved = false;
	SniperMoved = false;
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
	Agame_PlayerController* PlayerC = Cast<Agame_PlayerController>(UGameplayStatics::GetActorOfClass(GetWorld(), Agame_PlayerController::StaticClass()));
	if (PlayerC && PlayerC->HUD)
		PlayerC->HUD->SetTurnText(2);

	BrawlerAttacked = false;
	SniperAttacked = false;
	IsMyTurn = true;
	

	if (!(SniperPlaced && BrawlerPlaced))
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle1, [&]()
			{
				if (IsMyTurn)
				{
					AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
					AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
					int32 RandomNumber, Rand;
					Rand = -1;
					do {
						RandomNumber = FMath::RandRange(0, GameField->Size * GameField->Size - 1);
					} while (GameField->TileArray[RandomNumber]->GetTileStatus() == ETileStatus::OCCUPIED);

					UE_LOG(LogTemp, Warning, TEXT("Random index: %i"), RandomNumber);

					if (!(SniperPlaced || BrawlerPlaced))
					{
						Rand = FMath::RandRange(0, 1);
					}

					if (SniperPlaced || Rand == 0)
					{

						GameField->TileArray[RandomNumber]->SetTileStatus(2, ETileStatus::OCCUPIED);

						int32 X = RandomNumber / GameField->Size;
						int32 Y = RandomNumber % GameField->Size;

						FVector Position = GameField->GetRelativeLocationByXYPosition(X, Y);
						Position.Z = 1;
						AUnit* Unit = GameModality->SpawnCellUnit(2, Position, EPawnType::BRAWLER);
						BrawlerPlaced = true;
						if (Unit)
						{
							MyUnits.Add(Unit);
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("Null Unit"));
						}


					}
					else if (BrawlerPlaced || Rand == 1)
					{
						int32 X = RandomNumber / GameField->Size;
						int32 Y = RandomNumber % GameField->Size;

						FVector Position = GameField->GetRelativeLocationByXYPosition(X, Y);
						Position.Z = 1;
						AUnit* Unit = GameModality->SpawnCellUnit(2, Position, EPawnType::SNIPER);
						SniperPlaced = true;
						if (Unit)
						{
							MyUnits.Add(Unit);
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("Null Unit"));
						}


					}

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

		if (Brawler && Brawler->CanAttack())
		{
			GetWorld()->GetTimerManager().SetTimer(BrawlerAttackTimerHandle, this, &ARandomPlayer::HighlightAndAttackBrawler, waitTime, false);
			waitTime += 2.5;
			BrawlerAttacked = true;
		}

		if (Sniper && Sniper->CanAttack())
		{
			GetWorld()->GetTimerManager().SetTimer(SniperAttackTimerHandle, this, &ARandomPlayer::HighlightAndAttackSniper, waitTime, false);
			waitTime += 2.5;
			SniperAttacked = true;
		}

		if (RandomNumber == 0)
		{
			if (Brawler && !BrawlerAttacked) 
			{
				GetWorld()->GetTimerManager().SetTimer(BrawlerMoveTimerHandle, this, &ARandomPlayer::HighlightAndMoveBrawler, waitTime, false);
				waitTime += 2.5;
			}
			if (Sniper && !SniperAttacked)
			{
				GetWorld()->GetTimerManager().SetTimer(SniperMoveTimerHandle, this, &ARandomPlayer::HighlightAndMoveSniper, waitTime, false);
				waitTime += 2.5;
			}
		}
		else
		{
			if (Sniper && !SniperAttacked)
			{
				GetWorld()->GetTimerManager().SetTimer(SniperMoveTimerHandle, this, &ARandomPlayer::HighlightAndMoveSniper, waitTime, false);
				waitTime += 2.5;
			}
			if (Brawler && !BrawlerAttacked)
			{
				GetWorld()->GetTimerManager().SetTimer(BrawlerMoveTimerHandle, this, &ARandomPlayer::HighlightAndMoveBrawler, waitTime, false);
				waitTime += 2.5;
			}
		}
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
					if (Brawler && !BrawlerAttacked && Brawler->CanAttack())
					{
						GetWorld()->GetTimerManager().SetTimer(BrawlerAttackTimerHandle, this, &ARandomPlayer::HighlightAndAttackBrawler, wait, false);
						bool IsActive = GetWorld()->GetTimerManager().IsTimerActive(BrawlerAttackTimerHandle);
						UE_LOG(LogTemp, Warning, TEXT("BrawlerAttackTimerHandle active after SetTimer: %s"), IsActive ? TEXT("YES") : TEXT("NO"));
						wait += 2.5;
					}
					if (Sniper && !SniperAttacked && Sniper->CanAttack())
					{
						GetWorld()->GetTimerManager().SetTimer(SniperAttackTimerHandle, this, &ARandomPlayer::HighlightAndAttackSniper, wait, false);
						bool IsActive = GetWorld()->GetTimerManager().IsTimerActive(SniperAttackTimerHandle);
						UE_LOG(LogTemp, Warning, TEXT("SniperAttackTimerHandle active after SetTimer: %s"), IsActive ? TEXT("YES") : TEXT("NO"));
						wait += 2.5;
					}
				}
				else
				{
					if (Sniper && !SniperAttacked && Sniper->CanAttack())
					{
						GetWorld()->GetTimerManager().SetTimer(SniperAttackTimerHandle, this, &ARandomPlayer::HighlightAndAttackSniper, wait, false);
						wait += 2.5;
						bool IsActive = GetWorld()->GetTimerManager().IsTimerActive(SniperAttackTimerHandle);
						UE_LOG(LogTemp, Warning, TEXT("SniperAttackTimerHandle active after SetTimer: %s"), IsActive ? TEXT("YES") : TEXT("NO"));
					}
					if (Brawler && !BrawlerAttacked && Brawler->CanAttack())
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
	BrawlerAttacked = false;
	BrawlerMoved = false;
	BrawlerPlaced = false;

	SniperAttacked = false;
	SniperMoved = false;
	SniperPlaced = false;
}



int32 ARandomPlayer::CountStepsBFS(const FVector2D& Start, const FVector2D& Goal, AGameField* GF)
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

	// Per debug: stampa gli indici target
	/*FString TargetsStr;
	for (int32 idx : TargetIndices)
	{
		TargetsStr += TEXT("X:") + FString::FromInt(idx / GF->Size) + TEXT(" Y:") + FString::FromInt(idx % GF->Size) + TEXT(" ");
	}
	UE_LOG(LogTemp, Warning, TEXT("Target indices: %s"), *TargetsStr);*/

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
}

int32 ARandomPlayer::AStarSearch(const FVector2D& Start, const FVector2D& Goal, TArray<FVector2D>& Path)
{
	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	if (!GameField)
	{
		return -1;
	}

	struct FNode
	{
		FVector2D Position;
		int32 GCost; // Distanza dal punto di partenza
		int32 HCost; // Distanza stimata fino al goal (euristica)
		int32 FCost() const { return GCost + HCost; }
		FNode* Parent;

		FNode(FVector2D Pos, int32 G, int32 H, FNode* P = nullptr)
			: Position(Pos), GCost(G), HCost(H), Parent(P) {
		}
	};

	TMap<FVector2D, FNode*> OpenSet;
	TMap<FVector2D, FNode*> ClosedSet;
	TArray<FNode*> AllocatedNodes; // Per liberare memoria alla fine

	FNode* StartNode = new FNode(Start, 0, FMath::Abs(Start.X - Goal.X) + FMath::Abs(Start.Y - Goal.Y));
	OpenSet.Add(Start, StartNode);
	AllocatedNodes.Add(StartNode);

	while (OpenSet.Num() > 0)
	{
		// Trova il nodo con il costo F più basso
		FNode* CurrentNode = nullptr;
		for (auto& Pair : OpenSet)
		{
			if (!CurrentNode || Pair.Value->FCost() < CurrentNode->FCost())
			{
				CurrentNode = Pair.Value;
			}
		}

		if (!CurrentNode)
			break;

		FVector2D CurrentPos = CurrentNode->Position;
		OpenSet.Remove(CurrentPos);
		ClosedSet.Add(CurrentPos, CurrentNode);

		// Se raggiungiamo il Goal, ricostruiamo il percorso
		if (CurrentPos == Goal)
		{
			while (CurrentNode)
			{
				Path.Insert(CurrentNode->Position, 0);
				CurrentNode = CurrentNode->Parent;
			}

			int32 Cost = Path.Num() - 1;

			// Libera la memoria allocata
			for (FNode* Node : AllocatedNodes)
			{
				delete Node;
			}

			return Cost;
		}

		static const int32 DirX[4] = { 1, -1, 0, 0 };
		static const int32 DirY[4] = { 0, 0, 1, -1 };
		// Direzioni: destra, sinistra, giù, su
		for (int i = 0; i < 4; i++)
		{
			FVector2D NeighborPos = FVector2D(CurrentPos.X + DirX[i], CurrentPos.Y + DirY[i]);

			if (NeighborPos.X < 0 || NeighborPos.X >= GameField->Size || NeighborPos.Y < 0 || NeighborPos.Y >= GameField->Size)
				continue; // Fuori dalla griglia

			int32 NeighborIndex = static_cast<int32>(NeighborPos.X) * GameField->Size + static_cast<int32>(NeighborPos.Y);
			if (GameField->TileArray[NeighborIndex]->GetTileStatus() != ETileStatus::EMPTY)
				continue; // La Tile è occupata

			if (ClosedSet.Contains(NeighborPos))
				continue; // Già visitata

			int32 NewGCost = CurrentNode->GCost + 1; // Movimento orizzontale/verticale costa 1
			int32 NewHCost = FMath::Abs(NeighborPos.X - Goal.X) + FMath::Abs(NeighborPos.Y - Goal.Y);

			if (!OpenSet.Contains(NeighborPos) || NewGCost < OpenSet[NeighborPos]->GCost)
			{
				FNode* NeighborNode = new FNode(NeighborPos, NewGCost, NewHCost, CurrentNode);
				OpenSet.Add(NeighborPos, NeighborNode);
				AllocatedNodes.Add(NeighborNode);
			}
		}
	}

	// Libera la memoria se non è stato trovato un percorso
	for (FNode* Node : AllocatedNodes)
	{
		delete Node;
	}

	return -1; 
}

FVector2D ARandomPlayer::FindAStarDestination(AUnit* MovingUnit, AUnit* TargetUnit)
{
	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	if (!MovingUnit || !TargetUnit || !GameField)
	{
		UE_LOG(LogTemp, Error, TEXT("FindAStarDestination: Invalid parameters"));
		return MovingUnit ? MovingUnit->Position : FVector2D(-1, -1);
	}

	FVector2D Start = MovingUnit->Position;
	FVector2D Target = TargetUnit->Position;

	// Se l'unità è già adiacente al bersaglio, non serve muoversi
	if (FMath::Abs(Start.X - Target.X) + FMath::Abs(Start.Y - Target.Y) == 1)
	{
		return Start;
	}

	// Trova le celle adiacenti al target che sono libere
	TArray<FVector2D> TargetTiles;
	int32 TargetX = static_cast<int32>(Target.X);
	int32 TargetY = static_cast<int32>(Target.Y);
	static const int32 DirX[4] = { 1, -1, 0, 0 };
	static const int32 DirY[4] = { 0, 0, 1, -1 };

	for (int i = 0; i < 4; i++)
	{
		int32 nx = TargetX + DirX[i];
		int32 ny = TargetY + DirY[i];

		if (nx >= 0 && nx < GameField->Size && ny >= 0 && ny < GameField->Size)
		{
			int32 TileIndex = nx * GameField->Size + ny;
			if (GameField->TileArray[TileIndex]->GetTileStatus() == ETileStatus::EMPTY)
			{
				TargetTiles.Add(FVector2D(nx, ny));
			}
		}
	}

	// Se non ci sono celle libere intorno al target, fermati sulla posizione attuale
	if (TargetTiles.Num() == 0)
	{
		TargetTiles.Add(Target);
	}

	// Esegui A* per trovare il percorso migliore tra Start e una delle TargetTiles
	int32 BestCost = INT_MAX;
	TArray<FVector2D> BestPath;

	for (const FVector2D& GoalTile : TargetTiles)
	{
		TArray<FVector2D> Path;
		int32 Cost = AStarSearch(Start, GoalTile, Path);
		if (Cost >= 0 && Cost < BestCost)
		{
			BestCost = Cost;
			BestPath = Path;
		}
	}

	// Se non è stato trovato un percorso, ritorna la posizione attuale
	if (BestPath.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FindAStarDestination: No valid path found"));
		return Start;
	}

	// Se il percorso ottimo ha lunghezza minore o uguale a MaxSteps, restituisci il punto finale
	if (BestPath.Num() - 1 <= MovingUnit->MovementRange)
	{
		return BestPath.Last();
	}
	else
	{
		// Altrimenti, restituisci la cella raggiungibile dopo MaxSteps lungo il percorso
		return BestPath[MovingUnit->MovementRange];
	}
}

FString ARandomPlayer::GetCellString(const FVector2D& CellCoord)
{
	// Convertiamo la colonna in lettera: 0->A, 1->B, ...
	int32 Column = static_cast<int32>(CellCoord.X);
	TCHAR ColumnLetter = 'A' + Column;

	// Convertiamo la riga in numero (aggiungiamo 1 per usare 1-based indexing)
	int32 Row = static_cast<int32>(CellCoord.Y) + 1;

	// Combiniamo la lettera e il numero in una stringa
	return FString::Printf(TEXT("%c%d"), ColumnLetter, Row);
}


