// Fill out your copyright notice in the Description page of Project Settings.


#include "GameField.h"

#include "IContentBrowserSingleton.h"
#include "Kismet/GameplayStatics.h"
#include "GameModality.h"


// function to visit all the cells of the map
void DFS(int i, int j, int32 Size, const TArray<bool>& Obstacles, TArray<bool>& Visited) // depth-first search to check if the map is connected
{
	Visited[i * Size + j] = true;

	// array with the 4 possible directions
	const int32 DirX[4] = { 1, -1, 0, 0 };
	const int32 DirY[4] = { 0, 0, 1, -1 };

	for (int d = 0; d < 4; d++)
	{
		int nx = i + DirX[d];
		int ny = j + DirY[d];

		// limit check
		if (nx >= 0 && nx < Size && ny >= 0 && ny < Size)
		{
			int32 Index = nx * Size + ny;
			// if the cell is free and not visited, visit it
			if (!Obstacles[Index] && !Visited[Index])
			{
				DFS(nx, ny, Size, Obstacles, Visited);
			}
		}
	}
}

// function to check if the map is connected
bool isTotallyConnected(const TArray<bool>& Obstacles, int32 Size)
{
	// fiund the first cell with no obstacle
	int32 StartIndex = Obstacles.Find(false);
	if (StartIndex == INDEX_NONE)
	{
		// if all cells are obstacles, the map is connected
		return true;
	}

	// create a visited array
	TArray<bool> Visited;
	Visited.Init(false, Size * Size);

	// DFS
	int32 StartX = StartIndex / Size;
	int32 StartY = StartIndex % Size;
	DFS(StartX, StartY, Size, Obstacles, Visited);

	// check if all cells are visited
	for (int32 i = 0; i < Size * Size; i++)
	{
		if (!Obstacles[i] && !Visited[i])
		{
			return false; // if there is a cell that is not visited and not an obstacle, the map is not connected
		}
	}
	return true;
}


TArray<bool> AGameField::GenerateObstacles(float ObstaclePercentage) {
	TArray<bool> Obstacles; // map of obstacles (true if there is an obstacle)
	bool allConnected = true; // flag to check if the map is connected
	
	if (ObstaclePercentage < 0 || ObstaclePercentage > 1)
	{
		UE_LOG(LogTemp, Error, TEXT("ObstaclePercentage must be between 0 and 1"));
		return Obstacles;
	}

	
	Obstacles.Init(false, Size * Size); // initialize the map with all false
	int32 NumObstacles = FMath::RoundToInt(ObstaclePercentage * Size * Size);

	TSet<int32> UniqueObstacles;
	while (UniqueObstacles.Num() < NumObstacles)
	{
		int32 UniqueObstaclesNum = UniqueObstacles.Num();
		int32 RandomNumber = -1;
		while (UniqueObstacles.Num() == UniqueObstaclesNum) {
			RandomNumber = FMath::RandRange(0, Size * Size - 1);
			UniqueObstacles.Add(RandomNumber);
		}

		Obstacles[RandomNumber] = true;
			
		while (!isTotallyConnected(Obstacles, Size)) 
		{
			Obstacles[RandomNumber] = false;
			UniqueObstacles.Remove(RandomNumber);
			while (UniqueObstacles.Num() == UniqueObstaclesNum) {
				RandomNumber = FMath::RandRange(0, Size * Size - 1);
				UniqueObstacles.Add(RandomNumber);
			}
			Obstacles[RandomNumber] = true;
		}

	}


	return Obstacles;
}

// Sets default values
AGameField::AGameField()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	// size of winning line
	WinSize = 3;
	// size of the field (3x3)
	Size = 25;
	// tile dimension
	TileSize = 120.f;
	// tile padding percentage 
	CellPadding = 0.2f;
	
	// tile class
	static ConstructorHelpers::FClassFinder<ATile> TileClassFinder(TEXT("/Script/CoreUObject.Class'/Script/gioco.Tile'"));
	if (TileClassFinder.Succeeded())
	{
		TileClass = TileClassFinder.Class;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TileClass non trovata!"));
	}

}

void AGameField::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	//normalized tilepadding
	NextCellPositionMultiplier = (TileSize + TileSize * CellPadding) / TileSize;
}

// Called when the game starts or when spawned
void AGameField::BeginPlay()
{
	Super::BeginPlay();
	GenerateField();
}

void AGameField::ResetField()
{
	for (ATile* Obj : TileArray)
	{
		Obj->SetTileStatus(NOT_ASSIGNED, ETileStatus::EMPTY);
	}

	// send broadcast event to registered objects 
	OnResetEvent.Broadcast();

	AGameModality* GameMode = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
	GameMode->IsGameOver = false;
	GameMode->MoveCounter = 0;
	GameMode->ChoosePlayerAndStartGame();
}

void AGameField::GenerateField()
{

	
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("GetWorld() è NULL!"));
		return;
	}

	if (!TileClass)
	{
		UE_LOG(LogTemp, Error, TEXT("TileClass non è impostata!"));
		return;
	}

	TArray<bool> Obstacles = GenerateObstacles(ObstacleDensity);

	for (int32 i = 0; i < Size; i++)
	{
		for (int32 j = 0; j < Size; j++)
		{
			FVector Location = AGameField::GetRelativeLocationByXYPosition(i, j);
			ATile* Obj = GetWorld()->SpawnActor<ATile>(TileClass, Location, FRotator::ZeroRotator);
			if (!Obj)
			{
				UE_LOG(LogTemp, Warning, TEXT("Impossibile spawnare ATile a %s"), *Location.ToString());
				continue;
			}
			Obj->TileInit(Obstacles[i * Size + j]); // the tile is an obstacle if Obstacles[i * Size + j] is true
			const float TileScale = TileSize / 100.f;
			const float Zscaling = 0.2f;
			Obj->SetActorScale3D(FVector(TileScale, TileScale, Zscaling));
			Obj->SetGridPosition(i, j);
			TileArray.Add(Obj);
			TileMap.Add(FVector2D(i, j), Obj);
		}
	}
}

FVector2D AGameField::GetPosition(const FHitResult& Hit)
{
	return Cast<ATile>(Hit.GetActor())->GetGridPosition();
}

TArray<ATile*>& AGameField::GetTileArray()
{
	return TileArray;
}

FVector AGameField::GetRelativeLocationByXYPosition(const int32 InX, const int32 InY) const
{
	return TileSize * NextCellPositionMultiplier * FVector(InX, InY, 0);
}

FVector2D AGameField::GetXYPositionByRelativeLocation(const FVector& Location) const
{
	const double XPos = Location.X / (TileSize * NextCellPositionMultiplier);
	const double YPos = Location.Y / (TileSize * NextCellPositionMultiplier);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("x=%f,y=%f"), XPos, YPos));
	return FVector2D(XPos, YPos);
}

bool AGameField::IsWinPosition(const FVector2D Position) const
{
	const int32 Offset = WinSize - 1;
	// vertical lines
	for (int32 IndexI = Position.X - Offset; IndexI <= Position.X; IndexI++)
	{
		if (IsWinLine(FVector2D(IndexI, Position.Y), FVector2D(IndexI + Offset, Position.Y)))
		{
			return true;
		}
	}

	// horizontal lines
	for (int32 IndexI = Position.Y - Offset; IndexI <= Position[1]; IndexI++)
	{
		if (IsWinLine(FVector2D(Position.X, IndexI), FVector2D(Position.X, IndexI + Offset)))
		{
			return true;
		}
	}

	// diagonal lines
	for (int32 IndexI = -Offset; IndexI <= 0; IndexI++)
	{
		if (IsWinLine(FVector2D(Position.X + IndexI, Position.Y + IndexI), FVector2D(Position.X + Offset + IndexI, Position.Y + Offset + IndexI)))
		{
			return true;
		}
		if (IsWinLine(FVector2D(Position.X + IndexI, Position.Y - IndexI), FVector2D(Position.X + Offset + IndexI, Position.Y - Offset - IndexI)))
		{
			return true;
		}
	}

	return false;
}

inline bool AGameField::IsWinLine(const FVector2D Begin, const FVector2D End) const
{
	return IsValidPosition(Begin) && IsValidPosition(End) && AllEqual(GetLine(Begin, End));
}

inline bool AGameField::IsValidPosition(const FVector2D Position) const
{
	return 0 <= Position.X && Position.X < Size && 0 <= Position.Y && Position.Y < Size;
}

TArray<int32> AGameField::GetLine(const FVector2D Begin, const FVector2D End) const
{
	int32 XSign;
	if (Begin.X == End.X)
	{
		XSign = 0;
	}
	else
	{
		XSign = Begin.X < End.X ? 1 : -1;
	}

	int32 YSign;
	if (Begin.Y == End.Y)
	{
		YSign = 0;
	}
	else
	{
		YSign = Begin.Y < End.Y ? 1 : -1;
	}

	TArray<int32> Line;
	int32 XVal = Begin.X - XSign;
	int32 YVal = Begin.Y - YSign;
	do
	{
		XVal += XSign;
		YVal += YSign;
		Line.Add((TileMap[FVector2D(XVal, YVal)])->GetOwner());
	} while (XVal != End.X || YVal != End.Y);

	return Line;
}

bool AGameField::AllEqual(const TArray<int32>& Array) const
{
	if (Array.Num() == 0)
	{
		return false;
	}
	const int32 Value = Array[0];

	if (Value == NOT_ASSIGNED)
	{
		return false;
	}

	for (int32 IndexI = 1; IndexI < Array.Num(); IndexI++)
	{
		if (Value != Array[IndexI])
		{
			return false;
		}
	}

	return true;
}

// Called every frame
/*void AGameField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}*/

