// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomPlayer.h"

// Sets default values
ARandomPlayer::ARandomPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GameInstance = Cast<Ugame_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	BrawlerPlaced = false;
	SniperPlaced = false;
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

	FTimerHandle TimerHandle;

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
		{
			if (!(SniperPlaced && BrawlerPlaced))
			{
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
					int32 X = RandomNumber / GameField->Size;
					int32 Y = RandomNumber % GameField->Size;
			
					FVector Position = GameField->GetRelativeLocationByXYPosition(X, Y);
					Position.Z = 1;
					AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
					GameModality->SpawnCellUnit(2, Position, EPawnType::BRAWLER);

					FString LocationString = FString::Printf(TEXT("AI spawned a Brawler at the position (%i, %i)"), X, Y);
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, LocationString);

					GameModality->TurnNextPlayer();
				} 
				else if (BrawlerPlaced || Rand == 1)
				{
					int32 X = RandomNumber / GameField->Size;
					int32 Y = RandomNumber % GameField->Size;

					FVector Position = GameField->GetRelativeLocationByXYPosition(X, Y);
					Position.Z = 1;
					AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
					GameModality->SpawnCellUnit(2, Position, EPawnType::SNIPER);

					FString LocationString = FString::Printf(TEXT("AI spawned a Sniper at the position (%i, %i)"), X, Y);
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, LocationString);

					GameModality->TurnNextPlayer();
				}
				

			}
			
		}, 1.5, false);
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