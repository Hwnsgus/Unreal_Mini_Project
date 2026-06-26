// Copyright Epic Games, Inc. All Rights Reserved.

#include "Assign05HUDWidget.h"

#include "Assign05GameState.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UAssign05HUDWidget::RebuildWidget()
{
	if (WidgetTree && WidgetTree->RootWidget)
	{
		return Super::RebuildWidget();
	}

	const FSlateFontInfo LabelFont = FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 24);

	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(FMargin(24.0f, 18.0f, 24.0f, 0.0f))
		[
			SAssignNew(ScoreSlateText, STextBlock)
			.Text(FText::FromString(TEXT("Score: 0")))
			.Font(LabelFont)
			.ColorAndOpacity(FSlateColor(FLinearColor::White))
			.ShadowOffset(FVector2D(2.0f, 2.0f))
			.ShadowColorAndOpacity(FLinearColor::Black)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(FMargin(24.0f, 18.0f, 24.0f, 0.0f))
		[
			SAssignNew(TimerSlateText, STextBlock)
			.Text(FText::FromString(TEXT("Time: 0")))
			.Font(LabelFont)
			.ColorAndOpacity(FSlateColor(FLinearColor::White))
			.ShadowOffset(FVector2D(2.0f, 2.0f))
			.ShadowColorAndOpacity(FLinearColor::Black)
		];
}

void UAssign05HUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CacheDesignerWidgets();
	BindToGameState();
	RefreshFromGameState();
}

void UAssign05HUDWidget::NativeDestruct()
{
	if (CachedGameState)
	{
		CachedGameState->OnWaveStateChanged.RemoveDynamic(this, &UAssign05HUDWidget::RefreshFromGameState);
		CachedGameState = nullptr;
	}

	Super::NativeDestruct();
}

void UAssign05HUDWidget::CacheDesignerWidgets()
{
	ScoreText = Cast<UTextBlock>(GetWidgetFromName(TEXT("ScoreText")));
	TimerText = Cast<UTextBlock>(GetWidgetFromName(TEXT("TimerText")));
}

void UAssign05HUDWidget::BindToGameState()
{
	AAssign05GameState* AssignGameState = GetWorld() ? GetWorld()->GetGameState<AAssign05GameState>() : nullptr;
	if (AssignGameState == nullptr || AssignGameState == CachedGameState)
	{
		return;
	}

	if (CachedGameState)
	{
		CachedGameState->OnWaveStateChanged.RemoveDynamic(this, &UAssign05HUDWidget::RefreshFromGameState);
	}

	CachedGameState = AssignGameState;
	CachedGameState->OnWaveStateChanged.AddDynamic(this, &UAssign05HUDWidget::RefreshFromGameState);
}

void UAssign05HUDWidget::RefreshFromGameState()
{
	BindToGameState();

	const AAssign05GameState* AssignGameState = CachedGameState.Get();
	const FText NewScoreText = BuildScoreText(AssignGameState);
	const FText NewTimerText = BuildTimerText(AssignGameState);

	if (ScoreText)
	{
		ScoreText->SetText(NewScoreText);
	}
	if (TimerText)
	{
		TimerText->SetText(NewTimerText);
	}

	if (ScoreSlateText.IsValid())
	{
		ScoreSlateText->SetText(NewScoreText);
	}
	if (TimerSlateText.IsValid())
	{
		TimerSlateText->SetText(NewTimerText);
	}
}

FText UAssign05HUDWidget::BuildScoreText(const AAssign05GameState* AssignGameState) const
{
	const int32 Score = AssignGameState ? AssignGameState->GetScore() : 0;
	return FText::FromString(FString::Printf(TEXT("Score: %d"), Score));
}

FText UAssign05HUDWidget::BuildTimerText(const AAssign05GameState* AssignGameState) const
{
	const int32 TimeRemaining = AssignGameState ? FMath::CeilToInt(AssignGameState->GetTimeRemaining()) : 0;
	return FText::FromString(FString::Printf(TEXT("Time: %d"), TimeRemaining));
}
