// Copyright Epic Games, Inc. All Rights Reserved.

#include "Assign05StageTransitionWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Widget.h"
#include "Styling/CoreStyle.h"

void UAssign05StageTransitionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildWidgetTree();
	CacheDesignerWidgets();
	if (!HasDisplayWidgets())
	{
		BuildDefaultWidgetTree();
		CacheDesignerWidgets();
	}
	SetVisibility(ESlateVisibility::HitTestInvisible);
	SetRenderOpacity(0.0f);
}

void UAssign05StageTransitionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsShowing)
	{
		return;
	}

	ElapsedTime += InDeltaTime;

	float Opacity = 1.0f;
	if (ElapsedTime < FadeDuration)
	{
		Opacity = ElapsedTime / FadeDuration;
	}
	else if (ElapsedTime > CurrentDisplayDuration - FadeDuration)
	{
		Opacity = FMath::Max(0.0f, (CurrentDisplayDuration - ElapsedTime) / FadeDuration);
	}

	SetRenderOpacity(Opacity);

	if (AccentBar)
	{
		AccentBar->SetPercent(FMath::Clamp(ElapsedTime / CurrentDisplayDuration, 0.0f, 1.0f));
	}

	if (ElapsedTime >= CurrentDisplayDuration)
	{
		bIsShowing = false;
		RemoveFromParent();
	}
}

void UAssign05StageTransitionWidget::ShowStageTransition(int32 LevelNumber, int32 WaveNumber, float TimeLimit, int32 SpawnCount, int32 RequiredPickupCount)
{
	BuildWidgetTree();
	CacheDesignerWidgets();
	if (!HasDisplayWidgets())
	{
		BuildDefaultWidgetTree();
		CacheDesignerWidgets();
	}

	const FLinearColor StageColor = GetStageColor(LevelNumber);

	if (CardBorder)
	{
		CardBorder->SetBrushColor(FLinearColor(0.03f, 0.04f, 0.05f, 0.88f));
	}

	if (HeaderText)
	{
		SetDisplayText(HeaderText, HeaderTextBox, FText::FromString(TEXT("NEXT STAGE")));
		SetDisplayTextColor(HeaderText, HeaderTextBox, StageColor);
	}
	else if (HeaderTextBox)
	{
		SetDisplayText(nullptr, HeaderTextBox, FText::FromString(TEXT("NEXT STAGE")));
		SetDisplayTextColor(nullptr, HeaderTextBox, StageColor);
	}

	if (StageText || StageTextBox)
	{
		SetDisplayText(StageText, StageTextBox, FText::FromString(FString::Printf(TEXT("LEVEL %d  /  WAVE %d"), LevelNumber, WaveNumber)));
	}

	if (DetailText || DetailTextBox)
	{
		SetDisplayText(DetailText, DetailTextBox, FText::FromString(FString::Printf(TEXT("Time %.0fs  |  Items %d  |  Goal %d"), TimeLimit, SpawnCount, RequiredPickupCount)));
	}

	if (AccentBar)
	{
		AccentBar->SetFillColorAndOpacity(StageColor);
		AccentBar->SetPercent(0.0f);
	}

	ElapsedTime = 0.0f;
	CurrentDisplayDuration = DisplayDuration;
	bIsShowing = true;
	SetRenderOpacity(0.0f);
}

void UAssign05StageTransitionWidget::ShowGameClear(int32 FinalScore)
{
	BuildWidgetTree();
	CacheDesignerWidgets();
	if (!HasDisplayWidgets())
	{
		BuildDefaultWidgetTree();
		CacheDesignerWidgets();
	}

	const FLinearColor ClearColor(1.0f, 0.86f, 0.25f, 1.0f);

	if (CardBorder)
	{
		CardBorder->SetBrushColor(FLinearColor(0.03f, 0.04f, 0.05f, 0.92f));
	}

	if (HeaderText || HeaderTextBox)
	{
		SetDisplayText(HeaderText, HeaderTextBox, FText::FromString(TEXT("STAGE CLEAR")));
		SetDisplayTextColor(HeaderText, HeaderTextBox, ClearColor);
	}

	if (StageText || StageTextBox)
	{
		SetDisplayText(StageText, StageTextBox, FText::FromString(TEXT("ALL LEVELS CLEARED")));
	}

	if (DetailText || DetailTextBox)
	{
		SetDisplayText(DetailText, DetailTextBox, FText::FromString(FString::Printf(TEXT("Final Score  %d"), FinalScore)));
	}

	if (AccentBar)
	{
		AccentBar->SetFillColorAndOpacity(ClearColor);
		AccentBar->SetPercent(0.0f);
	}

	ElapsedTime = 0.0f;
	CurrentDisplayDuration = ClearDisplayDuration;
	bIsShowing = true;
	SetRenderOpacity(0.0f);
}

void UAssign05StageTransitionWidget::BuildWidgetTree()
{
	if (WidgetTree == nullptr)
	{
		return;
	}

	if (WidgetTree->RootWidget == nullptr)
	{
		BuildDefaultWidgetTree();
	}
}

void UAssign05StageTransitionWidget::BuildDefaultWidgetTree()
{
	if (WidgetTree == nullptr)
	{
		return;
	}

	CardBorder = nullptr;
	HeaderText = nullptr;
	HeaderTextBox = nullptr;
	StageText = nullptr;
	StageTextBox = nullptr;
	DetailText = nullptr;
	DetailTextBox = nullptr;
	AccentBar = nullptr;

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	UBorder* DimBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DimBackground"));
	DimBackground->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.35f));
	UCanvasPanelSlot* DimSlot = RootCanvas->AddChildToCanvas(DimBackground);
	DimSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
	DimSlot->SetOffsets(FMargin(0.0f));

	USizeBox* CardSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("CardSizeBox"));
	CardSizeBox->SetWidthOverride(560.0f);
	CardSizeBox->SetHeightOverride(260.0f);

	UCanvasPanelSlot* CardSlot = RootCanvas->AddChildToCanvas(CardSizeBox);
	CardSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	CardSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	CardSlot->SetAutoSize(true);

	CardBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("CardBorder"));
	CardBorder->SetPadding(FMargin(36.0f, 30.0f));
	CardBorder->SetBrushColor(FLinearColor(0.03f, 0.04f, 0.05f, 0.88f));
	CardSizeBox->SetContent(CardBorder);

	UVerticalBox* ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ContentBox"));
	CardBorder->SetContent(ContentBox);

	HeaderText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HeaderText"));
	HeaderText->SetJustification(ETextJustify::Center);
	HeaderText->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFont(), 22));
	ContentBox->AddChildToVerticalBox(HeaderText);

	StageText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StageText"));
	StageText->SetJustification(ETextJustify::Center);
	StageText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	StageText->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFont(), 36));
	if (UVerticalBoxSlot* StageSlot = ContentBox->AddChildToVerticalBox(StageText))
	{
		StageSlot->SetPadding(FMargin(0.0f, 20.0f, 0.0f, 14.0f));
	}

	DetailText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DetailText"));
	DetailText->SetJustification(ETextJustify::Center);
	DetailText->SetColorAndOpacity(FSlateColor(FLinearColor(0.78f, 0.82f, 0.88f, 1.0f)));
	DetailText->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFont(), 18));
	ContentBox->AddChildToVerticalBox(DetailText);

	AccentBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("AccentBar"));
	AccentBar->SetPercent(0.0f);
	if (UVerticalBoxSlot* BarSlot = ContentBox->AddChildToVerticalBox(AccentBar))
	{
		BarSlot->SetPadding(FMargin(10.0f, 28.0f, 10.0f, 0.0f));
	}
}

bool UAssign05StageTransitionWidget::HasDisplayWidgets() const
{
	return HeaderText != nullptr || HeaderTextBox != nullptr || StageText != nullptr || StageTextBox != nullptr || DetailText != nullptr || DetailTextBox != nullptr;
}

void UAssign05StageTransitionWidget::CacheDesignerWidgets()
{
	if (WidgetTree == nullptr)
	{
		return;
	}

	if (CardBorder == nullptr)
	{
		CardBorder = Cast<UBorder>(WidgetTree->FindWidget(TEXT("CardBorder")));
	}
	if (HeaderText == nullptr)
	{
		HeaderText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("HeaderText")));
	}
	if (HeaderTextBox == nullptr)
	{
		HeaderTextBox = Cast<UEditableTextBox>(WidgetTree->FindWidget(TEXT("HeaderText")));
	}
	if (StageText == nullptr)
	{
		StageText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("StageText")));
	}
	if (StageTextBox == nullptr)
	{
		StageTextBox = Cast<UEditableTextBox>(WidgetTree->FindWidget(TEXT("StageText")));
	}
	if (DetailText == nullptr)
	{
		DetailText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("DetailText")));
	}
	if (DetailTextBox == nullptr)
	{
		DetailTextBox = Cast<UEditableTextBox>(WidgetTree->FindWidget(TEXT("DetailText")));
	}
	if (AccentBar == nullptr)
	{
		AccentBar = Cast<UProgressBar>(WidgetTree->FindWidget(TEXT("AccentBar")));
	}

	TArray<UTextBlock*> TextBlocks;
	TArray<UEditableTextBox*> TextBoxes;
	TArray<UBorder*> Borders;
	TArray<UProgressBar*> ProgressBars;

	WidgetTree->ForEachWidget([&TextBlocks, &TextBoxes, &Borders, &ProgressBars](UWidget* Widget)
	{
		if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
		{
			TextBlocks.Add(TextBlock);
		}
		else if (UEditableTextBox* TextBox = Cast<UEditableTextBox>(Widget))
		{
			TextBox->SetIsReadOnly(true);
			TextBoxes.Add(TextBox);
		}
		else if (UBorder* Border = Cast<UBorder>(Widget))
		{
			Borders.Add(Border);
		}
		else if (UProgressBar* ProgressBar = Cast<UProgressBar>(Widget))
		{
			ProgressBars.Add(ProgressBar);
		}
	});

	if (HeaderText == nullptr && TextBlocks.IsValidIndex(0))
	{
		HeaderText = TextBlocks[0];
	}
	if (HeaderText == nullptr && HeaderTextBox == nullptr && TextBoxes.IsValidIndex(0))
	{
		HeaderTextBox = TextBoxes[0];
	}

	if (StageText == nullptr && TextBlocks.IsValidIndex(1))
	{
		StageText = TextBlocks[1];
	}
	if (StageText == nullptr && StageTextBox == nullptr && TextBoxes.IsValidIndex(1))
	{
		StageTextBox = TextBoxes[1];
	}

	if (DetailText == nullptr && TextBlocks.IsValidIndex(2))
	{
		DetailText = TextBlocks[2];
	}
	if (DetailText == nullptr && DetailTextBox == nullptr && TextBoxes.IsValidIndex(2))
	{
		DetailTextBox = TextBoxes[2];
	}

	if (CardBorder == nullptr && Borders.Num() > 0)
	{
		CardBorder = Borders[0];
	}

	if (AccentBar == nullptr && ProgressBars.Num() > 0)
	{
		AccentBar = ProgressBars[0];
	}
}

void UAssign05StageTransitionWidget::SetDisplayText(UTextBlock* TextBlock, UEditableTextBox* TextBox, const FText& Text) const
{
	if (TextBlock)
	{
		TextBlock->SetText(Text);
	}

	if (TextBox)
	{
		TextBox->SetText(Text);
		TextBox->SetIsReadOnly(true);
	}
}

void UAssign05StageTransitionWidget::SetDisplayTextColor(UTextBlock* TextBlock, UEditableTextBox* TextBox, const FLinearColor& Color) const
{
	if (TextBlock)
	{
		TextBlock->SetColorAndOpacity(FSlateColor(Color));
	}

	if (TextBox)
	{
		TextBox->SetForegroundColor(Color);
	}
}

FLinearColor UAssign05StageTransitionWidget::GetStageColor(int32 LevelNumber) const
{
	switch (LevelNumber)
	{
	case 1:
		return FLinearColor(0.35f, 0.70f, 1.0f, 1.0f);
	case 2:
		return FLinearColor(1.0f, 0.74f, 0.35f, 1.0f);
	default:
		return FLinearColor(0.35f, 0.95f, 0.58f, 1.0f);
	}
}
