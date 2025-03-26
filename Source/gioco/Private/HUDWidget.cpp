// Fill out your copyright notice in the Description page of Project Settings.


#include "HUDWidget.h"

bool UHUDWidget::Initialize()
{
	bool bSuccess = Super::Initialize();
	if (!bSuccess) return false;

    if (!WidgetTree)
    {
        UE_LOG(LogTemp, Error, TEXT("WidgetTree is null!"))
            return false;
    }

	// create a canvas as RootWidget
    RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
    if (!RootCanvas) 
    {
        UE_LOG(LogTemp, Error, TEXT("Cant put any root canvas!"))
    }
    WidgetTree->RootWidget = RootCanvas;

    // Creiamo il bottone Sniper
    SniperButton = NewObject<UButton>(this, UButton::StaticClass());
    if (SniperButton)
    {
        // Creiamo un testo per il bottone
        UTextBlock* SniperButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SniperText"));;
        if (SniperButtonText)
        {
            SniperButtonText->SetText(FText::FromString("Sniper"));
            SniperButton->AddChild(SniperButtonText);
        }
       
        // Aggiungiamo il bottone al canvas
        UCanvasPanelSlot* SniperSlot = RootCanvas->AddChildToCanvas(SniperButton);
        if (SniperSlot)
        {
            SniperSlot->SetPosition(FVector2D(50.f, 150.f));
            SniperSlot->SetSize(FVector2D(150.f, 50.f));
        }

        // Colleghiamo l'evento OnClicked
        if (AHumanPlayer* HP = Cast<AHumanPlayer>(GetOwningPlayerPawn()))
        {
            SniperButton->OnClicked.AddDynamic(HP, &AHumanPlayer::OnSniperButtonClicked);
        }
    }

    // Creiamo il bottone Brawler
    BrawlerButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("BrawlerButton"));
    if (BrawlerButton)
    {
        // Creiamo un testo per il bottone
        UTextBlock* BrawlerButtonText = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
        if (BrawlerButtonText)
        {
            BrawlerButtonText->SetText(FText::FromString("Brawler"));
            BrawlerButton->AddChild(BrawlerButtonText);
        }

        UCanvasPanelSlot* BrawlerSlot = RootCanvas->AddChildToCanvas(BrawlerButton);
        if (BrawlerSlot)
        {
            BrawlerSlot->SetPosition(FVector2D(50.f, 220.f));
            BrawlerSlot->SetSize(FVector2D(150.f, 50.f));
        }

        // OnClicked event
        // link the event
        if (AHumanPlayer* HP = Cast<AHumanPlayer>(GetOwningPlayerPawn()))
        {
            BrawlerButton->OnClicked.AddDynamic(HP, &AHumanPlayer::OnBrawlerButtonClicked);
        }
    }

    PassButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PassButton"));
    if (PassButton)
    {
        // Creiamo un testo per il bottone
        UTextBlock* PassButtonText = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
        if (PassButtonText)
        {
            PassButtonText->SetText(FText::FromString("Pass"));
            PassButton->AddChild(PassButtonText);
        }

        UCanvasPanelSlot* PassSlot = RootCanvas->AddChildToCanvas(PassButton);
        if (PassSlot)
        {
            PassSlot->SetPosition(FVector2D(50.f, 290.f));
            PassSlot->SetSize(FVector2D(150.f, 50.f));
        }

        // Evento OnClicked
        // Colleghiamo l'evento OnClicked
        if (AHumanPlayer* HP = Cast<AHumanPlayer>(GetOwningPlayerPawn()))
        {
            PassButton->OnClicked.AddDynamic(HP, &AHumanPlayer::OnPassButtonClicked);
        }
    }
    
    ResetButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ResetButton"));
    if (ResetButton)
    {
        // Creiamo un testo per il bottone
        UTextBlock* ResetButtonText = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
        if (ResetButtonText)
        {
            ResetButtonText->SetText(FText::FromString("Reset"));
            ResetButton->AddChild(ResetButtonText);
        }

        UCanvasPanelSlot* ResetSlot = RootCanvas->AddChildToCanvas(ResetButton);
        if (ResetSlot)
        {
            ResetSlot->SetPosition(FVector2D(50.f, 500.f));
            ResetSlot->SetSize(FVector2D(150.f, 50.f));
        }

        // Evento OnClicked
        // Colleghiamo l'evento OnClicked
        if (AHumanPlayer* HP = Cast<AHumanPlayer>(GetOwningPlayerPawn()))
        {
            ResetButton->OnClicked.AddDynamic(HP, &AHumanPlayer::OnResetButtonClicked);
        }
    }

    AddToViewport();

    return bSuccess;
}

void UHUDWidget::HideSniperButton()
{
	if (SniperButton)
	{
		SniperButton->SetVisibility(ESlateVisibility::Collapsed); // function to hide the button
	}
}

void UHUDWidget::HideBrawlerButton()
{
	if (BrawlerButton)
	{
		BrawlerButton->SetVisibility(ESlateVisibility::Collapsed); // function to hide the button
	}
}
