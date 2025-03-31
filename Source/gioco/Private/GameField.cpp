// Fill out your copyright notice in the Description page of Project Settings.


#include "GameField.h"

#include "IContentBrowserSingleton.h"
#include "Kismet/GameplayStatics.h"
#include "GameModality.h"
#include "HumanPlayer.h"
#include "RandomPlayer.h"


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

// method to generate obstacles 
// if the game field stops to be totally connected he removes the last obstacle inserted and sort another one
TArray<bool> AGameField::GenerateObstacles(float ObstaclePercentage) {
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

void AGameField::UnHighLight()
{
	for (int i = 0; i < Size; i++) 
	{
		for (int j = 0; j < Size; j++)
		{
			if (!TileArray[i * Size + j]->bIsObstacle)
			{
				TileArray[i * Size + j]->ResetTile(false);
			}
		}
	}
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
	// to reset i need to destroy all tiles and empty the array, then i can re-generate gamefield
	for (ATile* Tile : TileArray)
	{
		Tile->Destroy();
	}
	
	this->TileArray.Empty();

	GenerateField();

	// send broadcast event to registered objects 
	OnResetEvent.Broadcast();
	
	AGameModality* GameMode = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
	GameMode->IsGameOver = false;
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

	GenerateObstacles(ObstacleDensity);

	// after i generate properly obstacles he creates one by one all tiles 
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
			FRotator RotationIncrement = FRotator(0.f, 90.f, 0.f);
			Obj->AddActorLocalRotation(RotationIncrement);
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
	return FVector2D(XPos, YPos);
}



