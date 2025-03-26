// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Blueprint/WidgetTree.h"
#include "HumanPlayer.h"
#include "Components/CanvasPanelSlot.h"
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

	UPROPERTY()
	UCanvasPanel* RootCanvas;

	UFUNCTION(BlueprintCallable)
	void HideSniperButton();

	UFUNCTION(BlueprintCallable)
	void HideBrawlerButton();

};
