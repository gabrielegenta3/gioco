// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit.h"
#include "GameModality.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "game_GameInstance.h"
#include "HumanPlayer.h"
#include "RandomPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "game_PlayerController.h"

// Sets default values
AUnit::AUnit()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PawnType = EPawnType::NONE;
	StepTime = 0.2f;

	// template function that creates a components
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));

	// every actor has a RootComponent that defines the transform in the World
	SetRootComponent(Scene);
	StaticMeshComponent->SetupAttachment(Scene);

	// loading shape
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Script/Engine.StaticMesh'/Game/Shapes/Shape_Plane.Shape_Plane'"));
	if (MeshAsset.Succeeded())
	{
		StaticMeshComponent->SetStaticMesh(MeshAsset.Object);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Static Mesh non trovata nel costruttore!"));
	}

	// load the materials
	ConstructorHelpers::FObjectFinder<UMaterialInstance>* MatInstance = nullptr;
	MatInstance = new ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Materials/MI_AllyBrawler.MI_AllyBrawler'"));
	if (MatInstance && MatInstance->Succeeded())
	{
		AllyBrawler = MatInstance->Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AllyBrawler non trovata!"));
	}

	MatInstance = new ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Materials/MI_AllySniper.MI_AllySniper'"));
	if (MatInstance && MatInstance->Succeeded())
	{
		AllySniper = MatInstance->Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AllySniper non trovata!"));
	}

	MatInstance = new ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Materials/MI_EnemySniper.MI_EnemySniper'"));
	if (MatInstance && MatInstance->Succeeded())
	{
		EnemySniper = MatInstance->Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EnemySniper non trovata!"));
	}

	MatInstance = new ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Materials/MI_EnemyBrawler.MI_EnemyBrawler'"));
	if (MatInstance && MatInstance->Succeeded())
	{
		EnemyBrawler = MatInstance->Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EnemyBrawler non trovata!"));
	}

	delete MatInstance;
}

// Called when the game starts or when spawned
void AUnit::BeginPlay()
{
	Super::BeginPlay();
	AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
	GameModality->GameField->OnResetEvent.AddDynamic(this, &AUnit::SelfDestroy);
	
}

// method to assign basic values. this is used right after the unit spawn method
void AUnit::Init(EPawnType InPawnType, int32 InPlayerNumber, FVector2D Pos)
{
	PawnType = InPawnType;
	PlayerNumber = InPlayerNumber;
	Position = Pos;

	// setting Unit standard values
	if (PlayerNumber == 1)
	{
		if (PawnType == EPawnType::BRAWLER)
		{
			StaticMeshComponent->SetMaterial(0, AllyBrawler);
			HP = 40;
			MovementRange = 6;
			AttackRange = 1;
			MaxDamage = 6;
			MinDamage = 1;
		}
		else if (PawnType == EPawnType::SNIPER)
		{
			StaticMeshComponent->SetMaterial(0, AllySniper);
			HP = 20;
			MovementRange = 3;
			AttackRange = 10;
			MaxDamage = 8;
			MinDamage = 4;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PawnType non valido"));
		}
	}
	else if (PlayerNumber == 2)
	{
		if (PawnType == EPawnType::BRAWLER)
		{
			StaticMeshComponent->SetMaterial(0, EnemyBrawler);
			HP = 40;
			MovementRange = 6;
			AttackRange = 1;
			MaxDamage = 6;
			MinDamage = 1;
		}
		else if (PawnType == EPawnType::SNIPER)
		{
			StaticMeshComponent->SetMaterial(0, EnemySniper);
			HP = 20;
			MovementRange = 3;
			AttackRange = 10;
			MaxDamage = 8;
			MinDamage = 4;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PawnType non valido"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerNumber non valido!"));
	}
}

// method to check if the unit can attack based on his attack range
bool AUnit::CanAttack()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("GetWorld() is nullptr in CanAttack()"));
		return false;
	}

	AGameModality* GameModality = Cast<AGameModality>(World->GetAuthGameMode());
	if (!GameModality)
	{
		UE_LOG(LogTemp, Error, TEXT("GameModality is nullptr in CanAttack()"));
		return false;
	}

	// if the unit is of the human player it searches for the units of the random player, otherwise vice versa
	if (this->PlayerNumber == 1)
	{
		if (GameModality->Players.Num() <= 1)
		{
			UE_LOG(LogTemp, Error, TEXT("Players array does not have enough elements (expected at least 2)!"));
			return false;
		}

		ARandomPlayer* RandomPlayer = Cast<ARandomPlayer>(GameModality->Players[1]);
		if (!RandomPlayer)
		{
			UE_LOG(LogTemp, Error, TEXT("RandomPlayer is nullptr in CanAttack()"));
			return false;
		}

		for (AUnit* Unit : RandomPlayer->MyUnits)
		{
			if (!Unit) continue;

			int32 Distance = FMath::Abs(this->Position.X - Unit->Position.X) + FMath::Abs(this->Position.Y - Unit->Position.Y);
			if (Distance <= this->AttackRange)
			{
				UE_LOG(LogTemp, Warning, TEXT("Unit Can Attack"));
				return true;
			}
		}
	}
	else
	{
		if (GameModality->Players.Num() <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Players array is empty!"));
			return false;
		}

		AHumanPlayer* HumanPlayer = Cast<AHumanPlayer>(GameModality->Players[0]);
		if (!HumanPlayer)
		{
			UE_LOG(LogTemp, Error, TEXT("HumanPlayer is nullptr in CanAttack()"));
			return false;
		}

		for (AUnit* Unit : HumanPlayer->MyUnits)
		{
			if (!Unit) continue;

			int32 Distance = FMath::Abs(this->Position.X - Unit->Position.X) + FMath::Abs(this->Position.Y - Unit->Position.Y);
			if (Distance <= this->AttackRange)
			{
				if (this->PawnType == EPawnType::BRAWLER)
				{
					UE_LOG(LogTemp, Warning, TEXT("Brawler Can Attack"));
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Sniper Can Attack"));
				}
				return true;
			}
		}
	}

	if (this->PawnType == EPawnType::BRAWLER)
	{
		UE_LOG(LogTemp, Warning, TEXT("Brawler CANT Attack"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Sniper CANT Attack"));
	}
	return false;
}

// Called every frame
void AUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AUnit::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// method to self destroy the unit safely and removing himself from his owner's array
void AUnit::SelfDestroy()
{
	if (AGameField * GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass())))
	{
		if (this->PlayerNumber == 1)
		{
			AHumanPlayer* HumanPlayer = Cast<AHumanPlayer>(Cast<AGameModality>(GetWorld()->GetAuthGameMode())->Players[0]);
			if (HumanPlayer)
			{
				HumanPlayer->MyUnits.Remove(this);
				UE_LOG(LogTemp, Warning, TEXT("Tua unita rimossa"));
			}
		}
		else
		{
			ARandomPlayer* RandomPlayer = Cast<ARandomPlayer>(Cast<AGameModality>(GetWorld()->GetAuthGameMode())->Players[1]);
			if (RandomPlayer)
			{
				RandomPlayer->MyUnits.Remove(this);
				UE_LOG(LogTemp, Warning, TEXT("Unita nemica rimossa"));
			}
		}
		GameField->TileArray[this->Position.X * GameField->Size + this->Position.Y]->SetTileStatus(-1, ETileStatus::EMPTY);
		Destroy();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameField non trovato!"));
	}
	
}


void AUnit::FindPathAndMove(const FVector& Destination, AGameField* GameField)
{
	// Ensure that the GameField is valid before proceeding, preventing potential null pointer issues.
	if (!GameField)
	{
		UE_LOG(LogTemp, Error, TEXT("GameField è nullo!"));
		return;
	}

	// Get the starting index based on the unit's current position in the game field grid
	int32 StartIndex = static_cast<int32>(Position.X) * GameField->Size + static_cast<int32>(Position.Y);
	// Compute the goal index using the destination coordinates transformed into the game field grid
	int32 GoalIndex = GameField->GetXYPositionByRelativeLocation(Destination).X * GameField->Size + GameField->GetXYPositionByRelativeLocation(Destination).Y;

	// Use the Breadth-First Search (BFS) algorithm to find a path from the start to the goal index
	bool bFound = FindPathBFS(StartIndex, GoalIndex, GameField);
	if (!bFound)
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid path found!"));
		return;
	}

	// Convert the found path from grid indices to world-space positions
	TArray<FVector> WorldPositions = ConvertPathToWorldPositions(CurrentPath, GameField);

	// Trigger the unit's step-by-step movement along the computed path in world coordinates
	MoveAlongPath(WorldPositions);
}

// method to handle attack and eventually counter attack
void AUnit::Attack(AUnit* Target)
{
	int32 Damage = FMath::RandRange(this->MinDamage, this->MaxDamage);
	Target->TakeDamage(Damage);

	FString PlayerID = (PlayerNumber == 1) ? TEXT("HP") : TEXT("IA");
	FString UnitID = (PawnType == EPawnType::SNIPER) ? TEXT("S") : TEXT("B");
	FString Coordinates = GetCellString(Target->Position);
	Agame_PlayerController* PlayerC = Cast<Agame_PlayerController>(UGameplayStatics::GetActorOfClass(GetWorld(), Agame_PlayerController::StaticClass()));
	if (PlayerC && PlayerC->HUD)
	{
		PlayerC->HUD->AddTextToScrollBox(FString::Printf(TEXT("%s: %s %s %d"), *PlayerID, *UnitID, *Coordinates, Damage));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Player controller or HUD is null"));
	}

	// counter attack
	if (this->PawnType == EPawnType::SNIPER && (static_cast<int32>(FMath::Abs(this->Position.X - Target->Position.X) + FMath::Abs(this->Position.Y - Target->Position.Y)) <= Target->AttackRange)) {
		int32 CounterDamage = FMath::RandRange(MinCounter, MaxCounter);
		this->TakeDamage(CounterDamage);

		PlayerID = (Target->PlayerNumber == 1) ? TEXT("HP") : TEXT("IA");
		UnitID = (Target->PawnType == EPawnType::SNIPER) ? TEXT("S") : TEXT("B");
		Coordinates = GetCellString(this->Position);

		if (PlayerC && PlayerC->HUD)
		{
			PlayerC->HUD->AddTextToScrollBox(FString::Printf(TEXT("%s: %s %s %d"), *PlayerID, *UnitID, *Coordinates, CounterDamage));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Player controller or HUD is null"));
		}
	}

}

// method to handle damage taken
void AUnit::TakeDamage(const int32 Damage)
{
	if (Damage >= HP)
	{
		HP = 0;
		this->SelfDestroy();
	}
	else
	{
		HP -= Damage;
		
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
}




// Performs Breadth-First Search (BFS) to find a path (4 directions) from StartIndex to GoalIndex
bool AUnit::FindPathBFS(int32 StartIndex, int32 GoalIndex, AGameField* GF)
{
	if (StartIndex == GoalIndex)
	{
		// If already at the goal, return minimal path with only the start index
		CurrentPath = { (FVector)StartIndex };
		return true;
	}

	// Array to track where each index was reached from
	TArray<int32> CameFrom;
	CameFrom.Init(-1, GF->Size * GF->Size);

	// Queue for BFS traversal
	TQueue<int32> Queue;
	Queue.Enqueue(StartIndex);
	CameFrom[StartIndex] = StartIndex; // Root of the search tree

	bool bFoundGoal = false;

	// BFS loop to explore the grid
	while (!Queue.IsEmpty())
	{
		int32 Current;
		Queue.Dequeue(Current);

		// If goal is reached, stop searching
		if (Current == GoalIndex)
		{
			bFoundGoal = true;
			break;
		}

		// Compute grid coordinates from index
		int32 x = Current / GF->Size;
		int32 y = Current % GF->Size;

		// Define movement in 4 possible directions (right, left, down, up)
		static const int32 DirX[4] = { 1, -1, 0, 0 };
		static const int32 DirY[4] = { 0, 0, 1, -1 };

		// Explore neighbors in all 4 directions
		for (int i = 0; i < 4; i++)
		{
			int32 nx = x + DirX[i];
			int32 ny = y + DirY[i];

			// Check if the new position is within bounds
			if (nx >= 0 && nx < GF->Size && ny >= 0 && ny < GF->Size)
			{
				int32 NextIndex = nx * GF->Size + ny;

				// Ensure the tile is empty and hasn't been visited
				if (GF->TileArray[NextIndex]->GetTileStatus() == ETileStatus::EMPTY &&
					CameFrom[NextIndex] == -1)
				{
					CameFrom[NextIndex] = Current;
					Queue.Enqueue(NextIndex);
				}
			}
		}
	}

	// If no valid path was found, return false
	if (!bFoundGoal)
	{
		return false;
	}

	// Reconstruct the path from goal to start
	TArray<int32> PathIndices;
	int32 CurrentIndex = GoalIndex;
	while (CurrentIndex != StartIndex)
	{
		PathIndices.Add(CurrentIndex);
		CurrentIndex = CameFrom[CurrentIndex];
	}
	PathIndices.Add(StartIndex);
	Algo::Reverse(PathIndices);

	// Store the path as FVector indices for later conversion
	CurrentPath.Empty(PathIndices.Num());
	for (int32 idx : PathIndices)
	{
		// Temporarily store indices as FVector for later conversion
		CurrentPath.Add(FVector(idx, 0.f, 0.f));
	}

	return true;
}


// Converts indexes saved in CurrentPath to 3D positions
TArray<FVector> AUnit::ConvertPathToWorldPositions(const TArray<FVector>& PathIndices, AGameField* GF)
{
	TArray<FVector> WorldPositions;
	for (const FVector& FakeIndexVec : PathIndices)
	{
		int32 Index = (int32)FakeIndexVec.X; // extract index
		FVector WorldLoc = GF->TileArray[Index]->GetActorLocation();
		WorldLoc.Z = 1;
		WorldPositions.Add(WorldLoc);
	}
	return WorldPositions;
}

// Start step-by-step movement
void AUnit::MoveAlongPath(const TArray<FVector>& WorldPositions)
{
	if (WorldPositions.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("Path too short or nonexistent."));
		return;
	}

	CurrentPathIndex = 0;
	// Replace CurrentPath with real-world positions
	CurrentPath = WorldPositions;

	// Start from index 0 and move the unit
	SetActorLocation(CurrentPath[CurrentPathIndex]);

	// Start a timer that calls MoveStep
	Ugame_GameInstance* GameInstance = Cast<Ugame_GameInstance>(GetGameInstance());
	GetWorldTimerManager().SetTimer(MoveTimerHandle, this, &AUnit::MoveStep, StepTime, true);
}

// Function called by the timer to move one step
void AUnit::MoveStep()
{
	CurrentPathIndex++;
	if (CurrentPathIndex >= CurrentPath.Num())
	{
		// End of path
		UE_LOG(LogTemp, Warning, TEXT("Path completed!"));
		GetWorldTimerManager().ClearTimer(MoveTimerHandle);
		return;
	}

	// Move the unit to the next position
	SetActorLocation(CurrentPath[CurrentPathIndex]);
	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	Position = GameField->GetXYPositionByRelativeLocation(CurrentPath[CurrentPathIndex]);
	Ugame_GameInstance* GameInstance = Cast<Ugame_GameInstance>(GetGameInstance());
	GameInstance->bIsMoving = false;
}

FString AUnit::GetCellString(const FVector2D& CellCoord)
{
	// Convert column index to letter: 0->A, 1->B, ...
	int32 Column = static_cast<int32>(CellCoord.X);
	TCHAR ColumnLetter = 'A' + Column;

	// Convert row index to number (add 1 for 1-based indexing)
	int32 Row = static_cast<int32>(CellCoord.Y) + 1;

	// Combine letter and number into a string
	return FString::Printf(TEXT("%c%d"), ColumnLetter, Row);
}

