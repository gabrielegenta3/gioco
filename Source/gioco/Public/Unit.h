// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerInterface.h"
#include "GameField.h"
#include "Unit.generated.h"


UCLASS()
class GIOCO_API AUnit : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AUnit();

	// calling to move a unit to a destination
	void FindPathAndMove(const FVector& Destination, AGameField* GameField);

	// method to attack another unit
	void Attack(AUnit* Target);

	// method to handle the damage you can take
	void TakeDamage(const int32 Damage);
protected:
	// constants
	static constexpr int32 MinCounter = 1;
	static constexpr int32 MaxCounter = 3;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// finds CurrentPath
	bool FindPathBFS(int32 StartIndex, int32 GoalIndex, AGameField* GF);

	// convert tile indeces to 3D positions
	TArray<FVector> ConvertPathToWorldPositions(const TArray<FVector>& PathIndices, AGameField* GF);

	// starts moving step to step
	void MoveAlongPath(const TArray<FVector>& WorldPositions);

	//function called by timer to move
	UFUNCTION()
	void MoveStep();

public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EPawnType PawnType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 PlayerNumber;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 MovementRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 AttackRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 MaxDamage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 MinDamage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 HP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D Position;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMeshComponent;

	// Material instance for the tile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInstance* AllyBrawler;

	// Material instance for the tile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInstance* AllySniper;

	// Material instance for the tile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInstance* EnemyBrawler;

	// Material instance for the tile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInstance* EnemySniper;

	// called to initialize the unit
	void Init(EPawnType InPawnType, int32 InPlayerNumber, FVector2D Pos);

	// method to check if the unit has any enemy at attack range
	bool CanAttack();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// destructor
	UFUNCTION()
	void SelfDestroy();

private:
	// Path in 3D positions
	UPROPERTY()
	TArray<FVector> CurrentPath;

	// Current index in the path
	int32 CurrentPathIndex;

	// Timer to manage step-by-step movement
	FTimerHandle MoveTimerHandle;

	// movement speed (whether you want to move in bursts or at a fixed tempo)
	float StepTime;
};
