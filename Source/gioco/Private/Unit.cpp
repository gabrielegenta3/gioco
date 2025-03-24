// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit.h"
#include "GameModality.h"
#include "GameField.h"

// Sets default values
AUnit::AUnit()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PawnType = EPawnType::NONE;

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
		}
		else if (PawnType == EPawnType::SNIPER)
		{
			StaticMeshComponent->SetMaterial(0, AllySniper);

			UE_LOG(LogTemp, Warning, TEXT("Settato il materiale AllySniper"));
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
		}
		else if (PawnType == EPawnType::SNIPER)
		{
			StaticMeshComponent->SetMaterial(0, EnemySniper);
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

