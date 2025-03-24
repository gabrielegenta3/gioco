// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile.h"
//#include "UObject/ConstructorHelpers.h"

// Sets default values
ATile::ATile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// template function that creates a components
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));

	// every actor has a RootComponent that defines the transform in the World
	SetRootComponent(Scene);
	StaticMeshComponent->SetupAttachment(Scene);

	Status = ETileStatus::EMPTY;
	PlayerOwner = -1;
	TileGridPosition = FVector2D(0, 0);
	bIsObstacle = false;

	// load the materials
	ConstructorHelpers::FObjectFinder<UMaterialInstance>* MatInstance = nullptr;
	MatInstance = new ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Materials/MI_Pine.MI_Pine'"));
	if (MatInstance && MatInstance->Succeeded())
	{
		PineTile = MatInstance->Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Pine Tile non trovata!"));
	}

	MatInstance = new ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Materials/MI_Tree.MI_Tree'"));
	if (MatInstance && MatInstance->Succeeded())
	{
		TreeTile = MatInstance->Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Tree Tile non trovata!"));
	}

	MatInstance = new ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Materials/MI_Mountain.MI_Mountain'"));
	if (MatInstance && MatInstance->Succeeded())
	{
		MountainTile = MatInstance->Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Mountain Tile non trovata!"));
	}

	MatInstance = new ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Materials/MI_EmptyTile.MI_EmptyTile'"));
	if (MatInstance && MatInstance->Succeeded())
	{
		EmptyTile = MatInstance->Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Empty Tile non trovata!"));
	}

	MatInstance = new ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Materials/MI_Green.MI_Green'"));
	if (MatInstance && MatInstance->Succeeded())
	{
		GreenTile = MatInstance->Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Green Tile non trovata!"));
	}

	MatInstance = new ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Materials/MI_Red.MI_Red'"));
	if (MatInstance && MatInstance->Succeeded())
	{
		RedTile = MatInstance->Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Red Tile non trovata!"));
	}

	delete MatInstance;

	//SetRootComponent(StaticMeshComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Script/Engine.StaticMesh'/Game/StarterContent/Shapes/Shape_Plane.Shape_Plane'"));
	if (MeshAsset.Succeeded())
	{
		StaticMeshComponent->SetStaticMesh(MeshAsset.Object);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Static Mesh non trovata nel costruttore!"));
	}

}


void ATile::TileInit(bool isObstacle) {
	if (isObstacle) {		// load the material

		// set the status of the tile
		Status = ETileStatus::OCCUPIED;
		bIsObstacle = isObstacle;

		// random material
		int32 RandomNumber = FMath::RandRange(1, 3);

		if (RandomNumber == 1)
			StaticMeshComponent->SetMaterial(0, PineTile);
		else if (RandomNumber == 2)
			StaticMeshComponent->SetMaterial(0, TreeTile);
		else
			StaticMeshComponent->SetMaterial(0, MountainTile);
		
	}
	else {
		// load the material
		StaticMeshComponent->SetMaterial(0, EmptyTile);

	}
}



void ATile::SetTileStatus(const int32 TileOwner, const ETileStatus TileStatus)
{
	PlayerOwner = TileOwner;
	Status = TileStatus;
}

ETileStatus ATile::GetTileStatus()
{
	return Status;
}

int32 ATile::GetOwner()
{
	return PlayerOwner;
}

void ATile::SetGridPosition(const double InX, const double InY)
{
	TileGridPosition.Set(InX, InY);
}

FVector2D ATile::GetGridPosition()
{
	return TileGridPosition;
}

void ATile::LightUp()
{
	if (!bIsObstacle && Status == ETileStatus::OCCUPIED) 
	{
		StaticMeshComponent->SetMaterial(0, RedTile);
	}
	else if (!bIsObstacle)
	{
		StaticMeshComponent->SetMaterial(0, GreenTile);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("You can't light up a occupied tile!"));
	}
	
}

// Called when the game starts or when spawned
void ATile::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
/*void ATile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}*/

