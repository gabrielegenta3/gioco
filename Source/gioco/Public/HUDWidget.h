// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Blueprint/WidgetTree.h"
#include "HumanPlayer.h"
#include "RandomPlayer.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "HUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class GIOCO_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual bool Initialize() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Buttons")
	UButton* SniperButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Buttons")
	UButton* BrawlerButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Buttons")
	UButton* ResetButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Buttons")
	UButton* PassButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Buttons")
	UButton* EasyButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Buttons")
	UButton* HardButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Text")
	UTextBlock* TurnText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Text")
	UTextBlock* DifficultyText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Text")
	UTextBlock* WinLoseText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Text")
	UTextBlock* HumanUnitsText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Text")
	UTextBlock* RandomUnitsText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ScrollBox")
	UScrollBox* ScrollBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Image")
	UImage* BackgroundImage;

	UPROPERTY()
	UCanvasPanel* RootCanvas;

	UFUNCTION(BlueprintCallable)
	void HideSniperButton();

	UFUNCTION(BlueprintCallable)
	void HideBrawlerButton();

	UFUNCTION(BlueprintCallable)
	void HidePassButton();

	UFUNCTION(BlueprintCallable)
	void HideResetButton();

	UFUNCTION(BlueprintCallable)
	void HideHardButton();

	UFUNCTION(BlueprintCallable)
	void HideEasyButton();

	UFUNCTION(BlueprintCallable)
	void ShowPassButton();

	UFUNCTION(BlueprintCallable)
	void TurnIntoPlayHUD();

	UFUNCTION(BlueprintCallable)
	void TurnIntoMainMenuHUD();

	UFUNCTION(BlueprintCallable)
	void ShowWinLoseText(const FString& Message);

	UFUNCTION(BlueprintCallable)
	void AddTextToScrollBox(const FString& Message);

	UFUNCTION(BlueprintCallable)
	void ClearScrollBox();

	UFUNCTION(BlueprintCallable)
	void UpdateUnitHP();

	UFUNCTION(BlueprintCallable)
	void ClearUnitHP();

	UFUNCTION(BlueprintCallable)
	void SetTurnText(int32 PlayerNumber);
};
