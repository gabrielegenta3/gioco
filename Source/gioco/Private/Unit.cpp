// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit.h"
#include "GameModality.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AUnit::AUnit()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PawnType = EPawnType::NONE;
	StepTime = 0.5f;

	// template function that creates a components
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));

	// every actor has a RootComponent that defines the transform in the World
	SetRootComponent(Scene);
	StaticMeshComponent->SetupAttachment(Scene);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Script/Engine.StaticMesh'/Game/StarterContent/Shapes/Shape_Plane.Shape_Plane'"));
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

void AUnit::Init(EPawnType InPawnType, int32 InPlayerNumber, FVector2D Pos)
{
	PawnType = InPawnType;
	PlayerNumber = InPlayerNumber;
	Position = Pos;
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

void AUnit::SelfDestroy()
{
	Destroy();
}


void AUnit::FindPathAndMove(const FVector& Destination, AGameField* GameField)
{
	if (!GameField)
	{
		UE_LOG(LogTemp, Error, TEXT("GameField è nullo!"));
		return;
	}

	// Ottieni l'indice di partenza dall'unità
	int32 StartIndex = static_cast<int32>(Position.X) * GameField->Size + static_cast<int32>(Position.Y);
	// Ottieni l'indice di arrivo dalla destinazione
	int32 GoalIndex = GameField->GetXYPositionByRelativeLocation(Destination).X * GameField->Size + GameField->GetXYPositionByRelativeLocation(Destination).Y;

	// Esegui BFS
	bool bFound = FindPathBFS(StartIndex, GoalIndex, GameField);
	if (!bFound)
	{
		UE_LOG(LogTemp, Warning, TEXT("Nessun percorso trovato!"));
		return;
	}

	// Ora CurrentPath contiene gli indici
	// Converti in posizioni del mondo
	TArray<FVector> WorldPositions = ConvertPathToWorldPositions(CurrentPath, GameField);

	// Avvia il movimento passo-passo
	MoveAlongPath(WorldPositions);
}

// BFS to find a path (4 directions) from StartIndex to GoalIndex
bool AUnit::FindPathBFS(int32 StartIndex, int32 GoalIndex, AGameField* GF)
{
	if (StartIndex == GoalIndex)
	{
		// You're already on goal
		CurrentPath = { (FVector)StartIndex }; // Minimale, indica un solo indice
		return true;
	}

	// Array per tenere traccia da dove arrivi a ogni indice
	TArray<int32> CameFrom;
	CameFrom.Init(-1, GF->Size * GF->Size);

	TQueue<int32> Queue;
	Queue.Enqueue(StartIndex);
	CameFrom[StartIndex] = StartIndex; // radice di se stessa

	bool bFoundGoal = false;

	while (!Queue.IsEmpty())
	{
		int32 Current;
		Queue.Dequeue(Current);

		if (Current == GoalIndex)
		{
			bFoundGoal = true;
			break;
		}

		// Calcolo coordinate
		int32 x = Current / GF->Size;
		int32 y = Current % GF->Size;

		// 4 direzioni
		static const int32 DirX[4] = { 1, -1, 0, 0 };
		static const int32 DirY[4] = { 0, 0, 1, -1 };

		for (int i = 0; i < 4; i++)
		{
			int32 nx = x + DirX[i];
			int32 ny = y + DirY[i];

			if (nx >= 0 && nx < GF->Size && ny >= 0 && ny < GF->Size)
			{
				int32 NextIndex = nx * GF->Size + ny;

				// Controlla se la tile è libera e non visitata
				if (GF->TileArray[NextIndex]->GetTileStatus() == ETileStatus::EMPTY &&
					CameFrom[NextIndex] == -1)
				{
					CameFrom[NextIndex] = Current;
					Queue.Enqueue(NextIndex);
				}
			}
		}
	}

	if (!bFoundGoal)
	{
		return false;
	}

	// Ricostruisci il percorso
	TArray<int32> PathIndices;
	int32 CurrentIndex = GoalIndex;
	while (CurrentIndex != StartIndex)
	{
		PathIndices.Add(CurrentIndex);
		CurrentIndex = CameFrom[CurrentIndex];
	}
	PathIndices.Add(StartIndex);
	Algo::Reverse(PathIndices);

	// Salva in CurrentPath come indici (ma noi vogliamo le posizioni)
	// --> o usi un TArray<int32> se preferisci, ma qui semplifichiamo
	// Salviamo come "fake" in CurrentPath per ora
	CurrentPath.Empty(PathIndices.Num());
	for (int32 idx : PathIndices)
	{
		// Inseriamo come se fosse "FVector((float)idx, 0, 0)" momentaneo
		// Poi convertiremo in ConvertPathToWorldPositions
		CurrentPath.Add(FVector(idx, 0.f, 0.f));
	}

	return true;
}

// Converte gli indici salvati in CurrentPath in posizioni 3D
TArray<FVector> AUnit::ConvertPathToWorldPositions(const TArray<FVector>& PathIndices, AGameField* GF)
{
	TArray<FVector> WorldPositions;
	for (const FVector& FakeIndexVec : PathIndices)
	{
		int32 Index = (int32)FakeIndexVec.X; // estraiamo l'indice
		FVector WorldLoc = GF->TileArray[Index]->GetActorLocation();
		WorldLoc.Y = 1;
		WorldPositions.Add(WorldLoc);
	}
	return WorldPositions;
}

// Avvia il movimento passo-passo
void AUnit::MoveAlongPath(const TArray<FVector>& WorldPositions)
{
	if (WorldPositions.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("Percorso troppo corto o inesistente."));
		return;
	}

	CurrentPathIndex = 0;
	// Rimpiazziamo CurrentPath con posizioni reali
	CurrentPath = WorldPositions;

	// Iniziamo da 0, spostiamo l'unità
	SetActorLocation(CurrentPath[CurrentPathIndex]);

	// Avvia un timer che richiama MoveStep
	GetWorldTimerManager().SetTimer(MoveTimerHandle, this, &AUnit::MoveStep, StepTime, true);
}

// Funzione chiamata dal timer per spostarsi di uno step
void AUnit::MoveStep()
{
	CurrentPathIndex++;
	if (CurrentPathIndex >= CurrentPath.Num())
	{
		// Fine del percorso
		UE_LOG(LogTemp, Warning, TEXT("Percorso completato!"));
		GetWorldTimerManager().ClearTimer(MoveTimerHandle);
		return;
	}

	// Muoviamo l'unità alla prossima posizione
	SetActorLocation(CurrentPath[CurrentPathIndex]);
	AGameField* GameField = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	Position = GameField->GetXYPositionByRelativeLocation(CurrentPath[CurrentPathIndex]);

}