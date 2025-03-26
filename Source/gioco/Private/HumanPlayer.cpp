// Fill out your copyright notice in the Description page of Project Settings.


#include "HumanPlayer.h"
#include "HUDWidget.h"
#include "game_PlayerController.h"

// function to check in which tile a Unit could go based on their max step
// function to visit all the cells of the map
void BFSMovementRange(int32 startX, int32 startY, int32 size, int32 maxSteps, TArray<bool>& visited, AGameField* GF)
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
void BFSAttackRange(int32 startX, int32 startY, int32 size, int32 maxSteps, TArray<bool>& visited, AGameField* GF)
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
					if(!GF->TileArray[newIndex]->bIsObstacle && GF->TileArray[newIndex]->GetTileStatus() == ETileStatus::OCCUPIED && GF->TileArray[newIndex]->PlayerOwner == 2)
						GF->TileArray[newIndex]->LightUp();

					queue.Enqueue(FIntPoint(nx, ny));
					distanceQueue.Enqueue(dist + 1);
				}
			}
		}
	}
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

	SniperPlaced = false;
	BrawlerPlaced = false;
}

void AHumanPlayer::OnSniperButtonClicked()
{
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Sniper Button Clicked"));
	GameInstance->bSniperButtonClicked = true;
	GameInstance->bBrawlerButtonClicked = false;
}

void AHumanPlayer::OnBrawlerButtonClicked()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Brawler Button Clicked"));
	GameInstance->bBrawlerButtonClicked = true;
	GameInstance->bSniperButtonClicked = false;
}

void AHumanPlayer::OnResetButtonClicked()
{
}

void AHumanPlayer::OnPassButtonClicked()
{
	AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
	if (IsMyTurn)
	{
		GameModality->TurnNextPlayer();
	}
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
	IsMyTurn = true;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Your Turn"));	
}

void AHumanPlayer::OnWin()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("You Win!"));
}

void AHumanPlayer::OnLose()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("You Lose!"));
}

void AHumanPlayer::OnClick()
{
	// Structure containing information about the hit result
	FHitResult Hit = FHitResult(ForceInit);
	// GetHitResultUnderCursor function sends a ray from the mouse cursor on the screen and returns the hit result
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_Pawn, true, Hit);
	if (Hit.bBlockingHit && IsMyTurn && GameInstance->bSniperButtonClicked)
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
				GameModality->SpawnCellUnit(1, SpawnPosition, EPawnType::SNIPER);

				Agame_PlayerController* PlayerC = Cast<Agame_PlayerController>(UGameplayStatics::GetActorOfClass(GetWorld(), Agame_PlayerController::StaticClass()));
				PlayerC->HUD->HideSniperButton();

				AGameField* FoundField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
				FVector2D XYPosition = FoundField->GetXYPositionByRelativeLocation(SpawnPosition);
				FString LocationString = FString::Printf(TEXT("You spawned a sniper at the position (%i, %i)"), static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y));
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, LocationString);

				GameInstance->bSniperButtonClicked = false;
				SniperPlaced = true;

				if (BrawlerPlaced && SniperPlaced)
				{
					PlayerC->HUD->ShowPassButton();
				}
				
				GameModality->TurnNextPlayer();
			}
		}
	}
	else if (Hit.bBlockingHit && IsMyTurn && GameInstance->bBrawlerButtonClicked)
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
				GameModality->SpawnCellUnit(1, SpawnPosition, EPawnType::BRAWLER);

				Agame_PlayerController* PlayerC = Cast<Agame_PlayerController>(UGameplayStatics::GetActorOfClass(GetWorld(), Agame_PlayerController::StaticClass()));
				PlayerC->HUD->HideBrawlerButton();

				AGameField* FoundField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
				FVector2D XYPosition = FoundField->GetXYPositionByRelativeLocation(SpawnPosition);
				FString LocationString = FString::Printf(TEXT("You spawned a brawler at the position (%i, %i)"), static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y));
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, LocationString);

				GameInstance->bBrawlerButtonClicked = false;
				BrawlerPlaced = true;

				if (BrawlerPlaced && SniperPlaced)
				{
					PlayerC->HUD->ShowPassButton();
				}
				
				GameModality->TurnNextPlayer();
			}
		}
	}
	else if (Hit.bBlockingHit && IsMyTurn && GameInstance->bIsUnitClicked)
	{
		if (AUnit* CurrUnit = Cast<AUnit>(Hit.GetActor())) {
			AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));

			if (CurrUnit == GameInstance->SelectedUnit && GameField)
			{
				GameInstance->SelectedUnit = nullptr;
				GameField->UnHighLight();
				GameInstance->bIsUnitClicked = false;
			}
			else if (CurrUnit->PlayerNumber == 1 && GameField)
			{
				GameInstance->SelectedUnit = CurrUnit;
				GameField->UnHighLight();

				FVector2D XYPosition = CurrUnit->Position;
				TArray<bool> Visited;
				Visited.Init(false, GameField->Size * GameField->Size);
				BFSMovementRange(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y), GameField->Size, CurrUnit->MovementRange, Visited, GameField);
				Visited.Init(false, GameField->Size * GameField->Size);
				BFSAttackRange(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y), GameField->Size, CurrUnit->AttackRange, Visited, GameField);

			}
			else if (!GameField)
			{
				UE_LOG(LogTemp, Warning, TEXT("Couldn't cast the GameField"));
			}
		}
		else if(ATile* CurrTile = Cast<ATile>(Hit.GetActor()))
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
						GameInstance->SelectedUnit->FindPathAndMove(Destination, GameField);
						GameField->TileArray[Position.X * GameField->Size + Position.Y]->SetTileStatus(1, ETileStatus::OCCUPIED);
						GameField->UnHighLight();
					}
				}
			}
		}
	}
	else if (Hit.bBlockingHit && IsMyTurn)
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
					Visited.Init(false, GameField->Size * GameField->Size);
					BFSMovementRange(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y), GameField->Size, CurrUnit->MovementRange, Visited, GameField);
					BFSAttackRange(static_cast<int32>(XYPosition.X), static_cast<int32>(XYPosition.Y), GameField->Size, CurrUnit->AttackRange, Visited, GameField);
				}
				
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Couldn't cast the Unit you clicked on"));
		}
	}
}

