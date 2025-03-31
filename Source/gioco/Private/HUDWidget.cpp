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

    // Create a canvas as RootWidget
    RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
    if (!RootCanvas)
    {
        UE_LOG(LogTemp, Error, TEXT("Cant put any root canvas!"))
    }
    WidgetTree->RootWidget = RootCanvas;

    BackgroundImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("BackgroundImage"));
    if (BackgroundImage)
    {
        // Set the color with opacity (for example, semi-transparent black)
        BackgroundImage->SetColorAndOpacity(FColor(0, 119, 179).ReinterpretAsLinear());

        // Add the image to the canvas
        UCanvasPanelSlot* BackgroundSlot = RootCanvas->AddChildToCanvas(BackgroundImage);
        if (BackgroundSlot)
        {
            BackgroundSlot->SetAnchors(FAnchors(0, 0, 1, 1));
            BackgroundSlot->SetOffsets(FMargin(0));
        }
    }

    // To create sniper button
    SniperButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("SniperButton"));
    if (SniperButton)
    {
        // To create the button text
        UTextBlock* SniperButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SniperText"));;
        if (SniperButtonText)
        {
            SniperButtonText->SetText(FText::FromString("Sniper"));
            SniperButton->AddChild(SniperButtonText);
        }

        // Add the button to the canvas
        UCanvasPanelSlot* SniperSlot = RootCanvas->AddChildToCanvas(SniperButton);
        if (SniperSlot)
        {
            SniperSlot->SetPosition(FVector2D(50.f, 150.f));
            SniperSlot->SetSize(FVector2D(150.f, 50.f));
        }

        // Link the OnClicked event
        if (AHumanPlayer* HP = Cast<AHumanPlayer>(GetOwningPlayerPawn()))
        {
            SniperButton->OnClicked.AddDynamic(HP, &AHumanPlayer::OnSniperButtonClicked);
        }
    }

    // Create the Brawler button
    BrawlerButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("BrawlerButton"));
    if (BrawlerButton)
    {
        // Create text for the button
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
        // Link the event
        if (AHumanPlayer* HP = Cast<AHumanPlayer>(GetOwningPlayerPawn()))
        {
            BrawlerButton->OnClicked.AddDynamic(HP, &AHumanPlayer::OnBrawlerButtonClicked);
        }
    }

    PassButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PassButton"));
    if (PassButton)
    {
        // Create text for the button
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

        // OnClicked event
        // Link the OnClicked event
        if (AHumanPlayer* HP = Cast<AHumanPlayer>(GetOwningPlayerPawn()))
        {
            PassButton->OnClicked.AddDynamic(HP, &AHumanPlayer::OnPassButtonClicked);
        }
    }

    ResetButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ResetButton"));
    if (ResetButton)
    {
        // Create text for the button
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

        // OnClicked event
        // Link the OnClicked event
        if (AHumanPlayer* HP = Cast<AHumanPlayer>(GetOwningPlayerPawn()))
        {
            ResetButton->OnClicked.AddDynamic(HP, &AHumanPlayer::OnResetButtonClicked);
        }
    }

    ScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("ScrollBox"));
    if (ScrollBox)
    {
        UCanvasPanelSlot* ScrollSlot = RootCanvas->AddChildToCanvas(ScrollBox);
        if (ScrollSlot)
        {

            // Positioned at the bottom right
            ScrollSlot->SetAnchors(FAnchors(0.9f, 0.9f, 0.9f, 0.9f));
            ScrollSlot->SetAlignment(FVector2D(1.f, 1.f)); // Aligned to the bottom-right corner

            ScrollSlot->SetSize(FVector2D(300, 300));

            // Keeps the scrollbar visible
            ScrollBox->SetScrollBarVisibility(ESlateVisibility::Visible);
        }
    }

    TurnText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TurnText"));

    if (TurnText)
    {
        UCanvasPanelSlot* TurnTextSlot = RootCanvas->AddChildToCanvas(TurnText);

        if (TurnTextSlot)
        {
            float TextWidth = 300.f;
            float TextHeight = 30.f;

            TurnTextSlot->SetAnchors(FAnchors(0.02f, 0.9f, 0.1f, 0.9f));
            TurnTextSlot->SetAlignment(FVector2D(1.f, 1.f));
            TurnTextSlot->SetSize(FVector2D(TextWidth, TextHeight));
        }
    }


    // Create the TextBlock for HP
    HumanUnitsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HumanUnits"));
    RandomUnitsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RandomUnits"));

    if (HumanUnitsText && RandomUnitsText)
    {
        UCanvasPanelSlot* HumanHPSlot = RootCanvas->AddChildToCanvas(HumanUnitsText);
        UCanvasPanelSlot* RandomHPSlot = RootCanvas->AddChildToCanvas(RandomUnitsText);

        if (HumanHPSlot && RandomHPSlot)
        {
            float TextWidth = 300.f;
            float TextHeight = 30.f;

            HumanHPSlot->SetAnchors(FAnchors(0.9f, 0.1f, 0.9f, 0.1f));
            HumanHPSlot->SetAlignment(FVector2D(1.f, 1.f));
            HumanHPSlot->SetSize(FVector2D(TextWidth, TextHeight));

            RandomHPSlot->SetAnchors(FAnchors(0.9f, 0.25f, 0.9f, 0.25f));
            RandomHPSlot->SetAlignment(FVector2D(1.f, 1.f));
            RandomHPSlot->SetSize(FVector2D(TextWidth, TextHeight));
        }
    }

    // ------------------------------------------------------
    // Setup for the main menu
    DifficultyText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DifficultyText"));
    if (DifficultyText)
    {
        // Set the text
        DifficultyText->SetText(FText::FromString("Select the CPU difficulty to choose the beginner and start"));
        DifficultyText->SetJustification(ETextJustify::Center);
        DifficultyText->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 40));

        // Add the text block to the Canvas
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
            TextSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f)); 
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

// hiding and showing widget based on what I need in my PlayHUD
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
    if (ScrollBox)
    {
        ScrollBox->SetVisibility(ESlateVisibility::Visible);
    }
    if (HumanUnitsText)
    {
        HumanUnitsText->SetVisibility(ESlateVisibility::Visible);
    }
    if (RandomUnitsText)
    {
        RandomUnitsText->SetVisibility(ESlateVisibility::Visible);
    }
    if (TurnText)
    {
        TurnText->SetVisibility(ESlateVisibility::Visible);
    }

    if (WinLoseText)
    {
        WinLoseText->SetVisibility(ESlateVisibility::Collapsed);
    }
    
}

// hiding and showing widget based on what I need in my MainMenuHUD
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
    if (ScrollBox)
    {
        ScrollBox->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (HumanUnitsText)
    {
        HumanUnitsText->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (RandomUnitsText)
    {
        RandomUnitsText->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (TurnText)
    {
        TurnText->SetVisibility(ESlateVisibility::Collapsed);
    }
}

// hiding and showing widget based on what I need while showing my WinLoseText
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
    if (ScrollBox)
    {
        ScrollBox->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (HumanUnitsText)
    {
        HumanUnitsText->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (RandomUnitsText)
    {
        RandomUnitsText->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (TurnText)
    {
        TurnText->SetVisibility(ESlateVisibility::Collapsed);
    }
}

// method to easily add TextBlox to scroll bar
void UHUDWidget::AddTextToScrollBox(const FString& Message)
{
    if (ScrollBox)
    {
        UTextBlock* NewTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        if (NewTextBlock)
        {
            NewTextBlock->SetText(FText::FromString(Message));
            // add new text block to scrollbox
            ScrollBox->AddChild(NewTextBlock);
            ScrollBox->ScrollToEnd();
        }
    }
}

void UHUDWidget::ClearScrollBox()
{
    if (ScrollBox)
    {
        ScrollBox->ClearChildren();
    }
}

// method to check all units HP and update my Widget
void UHUDWidget::UpdateUnitHP()
{
    AHumanPlayer* HumanPlayer = Cast<AHumanPlayer>(Cast<AGameModality>(GetWorld()->GetAuthGameMode())->Players[0]);
    AUnit* Brawler = nullptr, * Sniper = nullptr;
    for (AUnit* Unit : HumanPlayer->MyUnits)
    {
        if (Unit->PawnType == EPawnType::BRAWLER)
            Brawler = Unit;
        else
            Sniper = Unit;
    }
    FString MainText = TEXT("Human Units:\n");
    FString BrawlerHP = TEXT("");
    FString SniperHP = TEXT("");

    if (Brawler)
        BrawlerHP = FString::Printf(TEXT("Brawler: %d\n"), Brawler->HP);
    if (Sniper)
        SniperHP = FString::Printf(TEXT("Sniper: %d"), Sniper->HP);

    HumanUnitsText->SetText(FText::FromString(FString::Printf(TEXT("%s%s%s"), *MainText, *BrawlerHP, *SniperHP)));
    
    
    ARandomPlayer* RandomPlayer = Cast<ARandomPlayer>(Cast<AGameModality>(GetWorld()->GetAuthGameMode())->Players[1]);
    Brawler = nullptr, Sniper = nullptr;
    for (AUnit* Unit : RandomPlayer->MyUnits)
    {
        if (Unit->PawnType == EPawnType::BRAWLER)
            Brawler = Unit;
        else
            Sniper = Unit;
    }
    MainText = TEXT("Random Units:\n");
    BrawlerHP = TEXT("");
    SniperHP = TEXT("");

    if (Brawler)
        BrawlerHP = FString::Printf(TEXT("Brawler: %d\n"), Brawler->HP);
    if (Sniper)
        SniperHP = FString::Printf(TEXT("Sniper: %d"), Sniper->HP);

    RandomUnitsText->SetText(FText::FromString(FString::Printf(TEXT("%s%s%s"), *MainText, *BrawlerHP, *SniperHP)));
    
}

void UHUDWidget::ClearUnitHP()
{
    HumanUnitsText->SetText(FText::FromString(FString::Printf(TEXT(""))));
    RandomUnitsText->SetText(FText::FromString(FString::Printf(TEXT(""))));
}

// setting turn text based on whose turn it is
void UHUDWidget::SetTurnText(int32 PlayerNumber)
{
    if (PlayerNumber == 1)
    {
        FSlateColor Blue = FSlateColor(FLinearColor::Blue);
        TurnText->SetText(FText::FromString(FString::Printf(TEXT("Your turn"))));
        TurnText->SetColorAndOpacity(Blue);
    }
    else
    {
        FSlateColor Red = FSlateColor(FLinearColor::Red);
        TurnText->SetText(FText::FromString(FString::Printf(TEXT("IA turn"))));
        TurnText->SetColorAndOpacity(Red);
    }
}
