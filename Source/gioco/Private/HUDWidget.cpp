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

    BackgroundImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("BackgroundImage"));
    if (BackgroundImage)
    {
        // Imposta il colore con opacità (ad esempio, nero semitrasparente)
        BackgroundImage->SetColorAndOpacity(FColor(0, 119, 179).ReinterpretAsLinear());

        // Aggiungi l'immagine al canvas
        UCanvasPanelSlot* BackgroundSlot = RootCanvas->AddChildToCanvas(BackgroundImage);
        if (BackgroundSlot)
        {
            BackgroundSlot->SetAnchors(FAnchors(0, 0, 1, 1));
            BackgroundSlot->SetOffsets(FMargin(0));
        }
    }

    // Creiamo il bottone Sniper
    SniperButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("SniperButton"));
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

    // ------------------------------------------------------
    // Setup for the main menu
    DifficultyText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DifficultyText"));
    if (DifficultyText)
    {
        // Imposta il testo
        DifficultyText->SetText(FText::FromString("Choose CPU difficulty to choose the beginner and start"));
		DifficultyText->SetJustification(ETextJustify::Center);
		DifficultyText->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 40));

        // Aggiungi il text block al Canvas
        UCanvasPanelSlot* TextSlot = RootCanvas->AddChildToCanvas(DifficultyText);
        if (TextSlot)
        {
            TextSlot->SetAnchors(FAnchors(0.5f, 0.f, 0.5f, 0.f));
            TextSlot->SetAlignment(FVector2D(0.5f, 0.f));
            TextSlot->SetPosition(FVector2D(0.f, 150.f));
            TextSlot->SetSize(FVector2D(400.f, 50.f));
        }
    }

    UHorizontalBox* ButtonBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ButtonBox"));
    if (ButtonBox)
    {
        if (UCanvasPanelSlot* ButtonBoxSlot = RootCanvas->AddChildToCanvas(ButtonBox))
        {
            ButtonBoxSlot->SetAnchors(FAnchors(0.5f, 0.f, 0.5f, 0.f));
            ButtonBoxSlot->SetAlignment(FVector2D(0.5f, 0.f));
            ButtonBoxSlot->SetPosition(FVector2D(0.f, 250.f));
            ButtonBoxSlot->SetSize(FVector2D(500.f, 100.f));
        }

        HardButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("HardButton"));
        if (HardButton)
        {
            if (UHorizontalBoxSlot* HardButtonSlot = ButtonBox->AddChildToHorizontalBox(HardButton))
            {
                HardButtonSlot->SetPadding(FMargin(10.f));
                HardButtonSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            }

            UTextBlock* HardButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HardButtonText"));
            if (HardButtonText)
            {
                HardButtonText->SetText(FText::FromString("Hard"));
                HardButton->AddChild(HardButtonText);
            }
            if (AHumanPlayer* HP = Cast<AHumanPlayer>(GetOwningPlayerPawn()))
            {
                HardButton->OnClicked.AddDynamic(HP, &AHumanPlayer::OnHardButtonClicked);
            }
        }

        EasyButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("EasyButton"));
        if (EasyButton)
        {
            if (UHorizontalBoxSlot* EasyButtonSlot = ButtonBox->AddChildToHorizontalBox(EasyButton))
            {
                EasyButtonSlot->SetPadding(FMargin(10.f));
                EasyButtonSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            }

            UTextBlock* EasyButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("EasyButtonText"));
            if (EasyButtonText)
            {
                EasyButtonText->SetText(FText::FromString("Easy"));
                EasyButton->AddChild(EasyButtonText);
            }
            if (AHumanPlayer* HP = Cast<AHumanPlayer>(GetOwningPlayerPawn()))
            {
                EasyButton->OnClicked.AddDynamic(HP, &AHumanPlayer::OnEasyButtonClicked);
            }
        }
    }


    // ------------------------------------------------------
	// Win/Lose/Tie Text
    WinLoseText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WinLoseText"));
    if (WinLoseText)
    {
        WinLoseText->SetText(FText::FromString(""));
        WinLoseText->SetVisibility(ESlateVisibility::Hidden); 
        WinLoseText->SetJustification(ETextJustify::Center);
        WinLoseText->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 180));

        UCanvasPanelSlot* TextSlot = RootCanvas->AddChildToCanvas(WinLoseText);
        if (TextSlot)
        {
            TextSlot->SetAnchors(FAnchors(0.5f, 0.4f, 0.5f, 0.4f)); 
            TextSlot->SetAlignment(FVector2D(0.5f, 0.5f));
            TextSlot->SetPosition(FVector2D(0.f, 0.f));
            TextSlot->SetSize(FVector2D(800.f, 200.f)); 
        }
    }

    AddToViewport();

	TurnIntoMainMenuHUD();

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

void UHUDWidget::HidePassButton()
{
    if (PassButton)
    {
        PassButton->SetVisibility(ESlateVisibility::Collapsed); // function to show the button
    }
}

void UHUDWidget::HideResetButton()
{
	if (ResetButton)
	{
		ResetButton->SetVisibility(ESlateVisibility::Collapsed); // function to show the button
	}
}

void UHUDWidget::HideHardButton()
{
	if (HardButton)
	{
		HardButton->SetVisibility(ESlateVisibility::Collapsed); // function to show the button
	}
}

void UHUDWidget::HideEasyButton()
{
	if (EasyButton)
	{
		EasyButton->SetVisibility(ESlateVisibility::Collapsed); // function to show the button
	}
}

void UHUDWidget::ShowPassButton()
{
    if (PassButton)
    {
        PassButton->SetVisibility(ESlateVisibility::Visible); // function to show the button
    }
}

void UHUDWidget::TurnIntoPlayHUD()
{
	if (DifficultyText)
	{
		DifficultyText->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (HardButton)
	{
		HardButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (EasyButton)
	{
		EasyButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (BackgroundImage)
	{
		BackgroundImage->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (SniperButton)
	{
		SniperButton->SetVisibility(ESlateVisibility::Visible);
	}
	if (BrawlerButton)
	{
		BrawlerButton->SetVisibility(ESlateVisibility::Visible);
	}
	if (ResetButton)
	{
		ResetButton->SetVisibility(ESlateVisibility::Visible);
	}
    if (WinLoseText)
    {
        WinLoseText->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UHUDWidget::TurnIntoMainMenuHUD()
{
	if (DifficultyText)
	{
		DifficultyText->SetVisibility(ESlateVisibility::Visible);
	}
	if (HardButton)
	{
		HardButton->SetVisibility(ESlateVisibility::Visible);
	}
	if (EasyButton)
	{
		EasyButton->SetVisibility(ESlateVisibility::Visible);
	}
    if (BackgroundImage)
    {
        BackgroundImage->SetVisibility(ESlateVisibility::Visible);
    }
	if (SniperButton)
	{
		SniperButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (BrawlerButton)
	{
		BrawlerButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ResetButton)
	{
		ResetButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (PassButton)
	{
		PassButton->SetVisibility(ESlateVisibility::Collapsed);
	}
    if (WinLoseText)
    {
        WinLoseText->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UHUDWidget::ShowWinLoseText(const FString& Message)
{
	if (WinLoseText)
	{
		WinLoseText->SetText(FText::FromString(Message));
		WinLoseText->SetVisibility(ESlateVisibility::Visible);
	}
	if (BackgroundImage)
	{
		BackgroundImage->SetVisibility(ESlateVisibility::Visible);
	}

	if (SniperButton)
	{
		SniperButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (BrawlerButton)
	{
		BrawlerButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (HardButton)
	{
		HardButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (EasyButton)
	{
		EasyButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (DifficultyText)
	{
		DifficultyText->SetVisibility(ESlateVisibility::Collapsed);
	}
    if (PassButton)
    {
        PassButton->SetVisibility(ESlateVisibility::Collapsed);
    }

}
