// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomPlayer.h"
#include "HumanPlayer.h"


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
	BFSAttackRange(static_cast<int32>(Brawler->Position.X), static_cast<int32>(Brawler->Position.Y), GameField->Size, Brawler->AttackRange, Visited, GameField);

	GetWorldTimerManager().SetTimer(BrawlerMoveTimerHandler, this, &ARandomPlayer::MoveBrawler, 1.f, false);
	
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
	BFSAttackRange(static_cast<int32>(Sniper->Position.X), static_cast<int32>(Sniper->Position.Y), GameField->Size, Sniper->AttackRange, Visited, GameField);

	GetWorldTimerManager().SetTimer(SniperMoveTimerHandler, this, &ARandomPlayer::MoveSniper, 1.f, false);
}

void ARandomPlayer::MoveBrawler()
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
		if (Enemy && Enemy->PlayerNumber == 1)
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
	ATile* ClosestToSniper = nullptr;
	ATile* ClosestToBrawler = nullptr;
	int32 SniperDistance = INT_MAX, BrawlerDistance = INT_MAX;

	for (ATile* Tile : GameField->TileArray)
	{
		if (Tile->bIsGreen)
		{
			int32 X = GameField->GetXYPositionByRelativeLocation(Tile->GetActorLocation()).X;
			int32 Y = GameField->GetXYPositionByRelativeLocation(Tile->GetActorLocation()).Y;

			int32 DistanceToSniper = INT_MAX, DistanceToBrawler = INT_MAX;

			if (EnemySniper)
			{
				DistanceToSniper = FMath::Abs(X - static_cast<int32>(EnemySniper->Position.X)) + FMath::Abs(Y - static_cast<int32>(EnemySniper->Position.Y));
			}
			
			if (EnemyBrawler)
			{
				DistanceToBrawler = FMath::Abs(X - static_cast<int32>(EnemyBrawler->Position.X)) + FMath::Abs(Y - static_cast<int32>(EnemyBrawler->Position.Y));
			}

			if (DistanceToSniper < SniperDistance)
			{
				SniperDistance = DistanceToSniper;
				ClosestToSniper = Tile;
			}

			if (DistanceToBrawler < BrawlerDistance)
			{
				BrawlerDistance = DistanceToBrawler;
				ClosestToBrawler = Tile;
			}
		}
	}


	if (ClosestToSniper && (SniperDistance < BrawlerDistance || RandomNumber == 0) && (SniperDistance > 0))
	{
		XYPosition = GameField->GetXYPositionByRelativeLocation(ClosestToSniper->GetActorLocation());
	}
	else if (ClosestToBrawler && (SniperDistance > BrawlerDistance || RandomNumber == 1) && (BrawlerDistance > 0))
	{
		XYPosition = GameField->GetXYPositionByRelativeLocation(ClosestToBrawler->GetActorLocation());
	}

	if (!(XYPosition.X == -1 && XYPosition.Y == -1))
	{
		GameField->TileArray[static_cast<int32>(Brawler->Position.X) * GameField->Size + static_cast<int32>(Brawler->Position.Y)]->SetTileStatus(-1, ETileStatus::EMPTY);
		Brawler->FindPathAndMove(GameField->GetRelativeLocationByXYPosition(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y)), GameField);
		GameField->TileArray[static_cast<int32>(XYPosition.X) * GameField->Size + static_cast<int32>(XYPosition.Y)]->SetTileStatus(2, ETileStatus::OCCUPIED);
	}



	GameField->UnHighLight();
	BrawlerMoved = true;
}

void ARandomPlayer::MoveSniper()
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
		if (Enemy && Enemy->PlayerNumber == 1)
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
	ATile* ClosestToSniper = nullptr;
	ATile* ClosestToBrawler = nullptr;
	int32 SniperDistance = INT_MAX, BrawlerDistance = INT_MAX;

	for (ATile* Tile : GameField->TileArray)
	{
		if (Tile->bIsGreen)
		{
			int32 X = GameField->GetXYPositionByRelativeLocation(Tile->GetActorLocation()).X;
			int32 Y = GameField->GetXYPositionByRelativeLocation(Tile->GetActorLocation()).Y;

			int32 DistanceToSniper = INT_MAX, DistanceToBrawler = INT_MAX;

			if (EnemySniper)
			{
				DistanceToSniper = FMath::Abs(X - static_cast<int32>(EnemySniper->Position.X)) + FMath::Abs(Y - static_cast<int32>(EnemySniper->Position.Y));
			}

			if (EnemyBrawler)
			{
				DistanceToBrawler = FMath::Abs(X - static_cast<int32>(EnemyBrawler->Position.X)) + FMath::Abs(Y - static_cast<int32>(EnemyBrawler->Position.Y));
			}

			if (DistanceToSniper < SniperDistance)
			{
				SniperDistance = DistanceToSniper;
				ClosestToSniper = Tile;
			}

			if (DistanceToBrawler < BrawlerDistance)
			{
				BrawlerDistance = DistanceToBrawler;
				ClosestToBrawler = Tile;
			}
		}
	}
	
	if (ClosestToSniper && (SniperDistance < BrawlerDistance || RandomNumber == 0) && (SniperDistance > 0))
	{
		XYPosition = GameField->GetXYPositionByRelativeLocation(ClosestToSniper->GetActorLocation());
	}
	else if (ClosestToBrawler && (SniperDistance > BrawlerDistance || RandomNumber == 1) && (BrawlerDistance > 0))
	{
		XYPosition = GameField->GetXYPositionByRelativeLocation(ClosestToBrawler->GetActorLocation());
	}

	if (!(XYPosition.X == -1 && XYPosition.Y == -1))
	{
		GameField->TileArray[static_cast<int32>(Sniper->Position.X) * GameField->Size + static_cast<int32>(Sniper->Position.Y)]->SetTileStatus(-1, ETileStatus::EMPTY);
		Sniper->FindPathAndMove(GameField->GetRelativeLocationByXYPosition(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y)), GameField);
		GameField->TileArray[static_cast<int32>(XYPosition.X) * GameField->Size + static_cast<int32>(XYPosition.Y)]->SetTileStatus(2, ETileStatus::OCCUPIED);
	}
	
	

	GameField->UnHighLight();
	SniperMoved = true;
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
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AI (Random) Turn"));
	GameInstance->SetTurnMessage(TEXT("AI (Random) Turn"));
	

	if (!(SniperPlaced && BrawlerPlaced))
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle1, [&]()
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

					FString LocationString = FString::Printf(TEXT("AI spawned a Brawler at the position (%i, %i)"), X, Y);
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, LocationString);

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

					FString LocationString = FString::Printf(TEXT("AI spawned a Sniper at the position (%i, %i)"), X, Y);
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, LocationString);

					
				}
				GameModality->TurnNextPlayer();

		}, 1.5, false);
	
	}
	else
	{
		int32 RandomNumber = FMath::RandRange(0, 1);
		if (RandomNumber == 0)
		{
			GetWorldTimerManager().SetTimer(BrawlerMoveTimerHandler, this, &ARandomPlayer::HighlightAndMoveBrawler, 1.f, false);
			GetWorldTimerManager().SetTimer(SniperMoveTimerHandler, this, &ARandomPlayer::HighlightAndMoveSniper, 3.f, false);
		}
		else
		{
			GetWorldTimerManager().SetTimer(SniperMoveTimerHandler, this, &ARandomPlayer::HighlightAndMoveSniper, 1.f, false);
			GetWorldTimerManager().SetTimer(BrawlerMoveTimerHandler, this, &ARandomPlayer::HighlightAndMoveBrawler, 3.f, false);
		}
		
		
		AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
		GameModality->TurnNextPlayer();
		/*GetWorld()->GetTimerManager().SetTimer(TimerHandle1, [&]()
			{

				AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
				AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
				for (AUnit* Unit : MyUnits)
				{
					AHumanPlayer* HumanPlayer = Cast<AHumanPlayer>(GameModality->Players[0]);
					int32 SniperDistance = INT_MAX, BrawlerDistance = INT_MAX;
					AUnit* Sniper = nullptr;
					AUnit* Brawler = nullptr;
					for (AUnit* Enemy : HumanPlayer->MyUnits)
					{
						if (Enemy && Enemy->PlayerNumber == 1)
						{
							if (Enemy->PawnType == EPawnType::SNIPER)
							{
								SniperDistance = static_cast<int32>(FMath::Abs(Unit->Position.X - Enemy->Position.X) + FMath::Abs(Unit->Position.Y - Enemy->Position.Y));
								Sniper = Enemy;
							}
							else
							{
								BrawlerDistance = static_cast<int32>(FMath::Abs(Unit->Position.X - Enemy->Position.X) + FMath::Abs(Unit->Position.Y - Enemy->Position.Y));
								Brawler = Enemy;
							}
						}
					}

					int32 RandomNumber = FMath::RandRange(0, 1);

					if (Brawler && BrawlerDistance < SniperDistance || (BrawlerDistance == SniperDistance && RandomNumber == 0))
					{
						FVector2D XYPosition = Unit->Position;
						TArray<bool> Visited;
						TArray<ATile*> VisitableTiles;

						Visited.Init(false, GameField->Size * GameField->Size);
						BFSMovementRange(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y), GameField->Size, Unit->MovementRange, Visited, GameField);
						Visited.Init(false, GameField->Size * GameField->Size);
						BFSAttackRange(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y), GameField->Size, Unit->AttackRange, Visited, GameField);

						for (ATile* Tile : GameField->TileArray) {
							if (Tile->bIsGreen)
							{
								VisitableTiles.Add(Tile);
							}
						}

						int32 ClosestToEnemy = 0;
						int32 MinDistance = INT32_MAX;
						for (int32 i = 0; i < VisitableTiles.Num() - 1; i++)  // Calculating the closest tile to the enemy
						{
							int32 X = GameField->GetXYPositionByRelativeLocation(VisitableTiles[i]->GetActorLocation()).X;
							int32 Y = GameField->GetXYPositionByRelativeLocation(VisitableTiles[i]->GetActorLocation()).Y;
							int32 Distance = static_cast<int32>(FMath::Abs(X - Brawler->Position.X) + FMath::Abs(Y - Brawler->Position.Y));

							if (Distance < MinDistance)
							{
								ClosestToEnemy = i;
								MinDistance = Distance;
							}
						}

						FVector Destination = VisitableTiles[ClosestToEnemy]->GetActorLocation();
						
						GetWorldTimerManager().SetTimer(MoveTimerHandler, this, Move , 1.5f, false);
						
						if (Unit->PawnType == EPawnType::BRAWLER)
						{
							BrawlerMoved = true;
						}
						else if (Unit->PawnType == EPawnType::SNIPER)
						{
							SniperMoved = true;
						}
						if (Unit->CanAttack())
						{
							Visited.Init(false, GameField->Size* GameField->Size);
							BFSAttackRange(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y), GameField->Size, Unit->AttackRange, Visited, GameField);

							GetWorldTimerManager().SetTimer(TimerHandle2, [&]()
								{
									Unit->Attack(Brawler);
									GameField->UnHighLight();
								}, 3, false);

						}
					}
					else if (Sniper && BrawlerDistance > SniperDistance || (BrawlerDistance == SniperDistance && RandomNumber == 1))
					{
						FVector2D XYPosition = Unit->Position;
						TArray<bool> Visited;
						TArray<ATile*> VisitableTiles;

						Visited.Init(false, GameField->Size * GameField->Size);
						BFSMovementRange(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y), GameField->Size, Unit->MovementRange, Visited, GameField);
						Visited.Init(false, GameField->Size * GameField->Size);
						BFSAttackRange(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y), GameField->Size, Unit->AttackRange, Visited, GameField);

						for (ATile* Tile : GameField->TileArray) {
							if (Tile->bIsGreen)
							{
								VisitableTiles.Add(Tile);
							}
						}

						int32 ClosestToEnemy = 0;
						for (int32 i = 0; i < VisitableTiles.Num() - 1; i++)  // Calculating the closest tile to the enemy
						{
							int32 X = GameField->GetXYPositionByRelativeLocation(VisitableTiles[i]->GetActorLocation()).X;
							int32 Y = GameField->GetXYPositionByRelativeLocation(VisitableTiles[i]->GetActorLocation()).Y;
							int32 Distance = static_cast<int32>(FMath::Abs(X - Sniper->Position.X) + FMath::Abs(Y - Sniper->Position.Y));

							if (Distance < ClosestToEnemy)
							{
								ClosestToEnemy = i;
							}
						}

						FVector Destination = VisitableTiles[ClosestToEnemy]->GetActorLocation();
						GameField->TileArray[static_cast<int32>(Unit->Position.X) * GameField->Size + static_cast<int32>(Unit->Position.Y)]->SetTileStatus(-1, ETileStatus::EMPTY);
						FVector2D Position = GameField->GetXYPositionByRelativeLocation(Destination);
						Unit->FindPathAndMove(Destination, GameField);
						GameField->TileArray[static_cast<int32>(Position.X) * GameField->Size + static_cast<int32>(Position.Y)]->SetTileStatus(2, ETileStatus::OCCUPIED);
						GameField->UnHighLight();

						if (Unit->PawnType == EPawnType::BRAWLER)
						{
							BrawlerMoved = true;
						}
						else if (Unit->PawnType == EPawnType::SNIPER)
						{
							SniperMoved = true;
						}

						if (Unit->CanAttack())
						{
							Visited.Init(false, GameField->Size* GameField->Size);
							BFSAttackRange(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y), GameField->Size, Unit->AttackRange, Visited, GameField);

							GetWorldTimerManager().SetTimer(TimerHandle2, [&]()
								{
									Unit->Attack(Sniper);
									GameField->UnHighLight();
								}, 3, false);

						}
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("No valid enemy found"));
					}

				}

				GameModality->TurnNextPlayer();

			}, 1.5, false);*/
	}
}

void ARandomPlayer::OnWin()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AI (Random) Wins!"));
	GameInstance->SetTurnMessage(TEXT("AI Wins!"));
	GameInstance->IncrementScoreAiPlayer();
}

void ARandomPlayer::OnLose()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AI (Random) Loses!"));
	GameInstance->SetTurnMessage(TEXT("AI Loses!"));
}