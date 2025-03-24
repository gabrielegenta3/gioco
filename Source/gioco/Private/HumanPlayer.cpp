// Fill out your copyright notice in the Description page of Project Settings.


#include "HumanPlayer.h"

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



// Called when the game starts or when spawned
void AHumanPlayer::BeginPlay()
{
	Super::BeginPlay();
	
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
				SpawnPosition.Z += 10;
				AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
				GameModality->SpawnCellUnit(1, SpawnPosition, EPawnType::SNIPER);
				FString LocationString = FString::Printf(TEXT("You spawned a sniper at the position (%.0f, %.0f)"), SpawnPosition.X, SpawnPosition.Y);
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, LocationString);
				IsMyTurn = false;
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
				SpawnPosition.Z += 0.1;
				AGameModality* GameModality = Cast<AGameModality>(GetWorld()->GetAuthGameMode());
				GameModality->SpawnCellUnit(1, SpawnPosition, EPawnType::BRAWLER);
				FString LocationString = FString::Printf(TEXT("You spawned a brawler at the position (%.0f, %.0f)"), SpawnPosition.X, SpawnPosition.Y);
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, LocationString);
				IsMyTurn = false;
			}
		}
	}
	else if (Hit.bBlockingHit && IsMyTurn)
	{
		if (AUnit* CurrUnit = Cast<AUnit>(Hit.GetActor())) {
			if (CurrUnit->PlayerNumber == 1) 
			{

			}
		}
	}
}

