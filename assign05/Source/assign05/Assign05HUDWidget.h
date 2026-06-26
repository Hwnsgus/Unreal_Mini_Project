// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Assign05HUDWidget.generated.h"

class AAssign05GameState;
class STextBlock;
class UTextBlock;

UCLASS()
class ASSIGN05_API UAssign05HUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UFUNCTION()
	void RefreshFromGameState();

private:
	void CacheDesignerWidgets();
	void BindToGameState();
	FText BuildScoreText(const AAssign05GameState* AssignGameState) const;
	FText BuildTimerText(const AAssign05GameState* AssignGameState) const;

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY()
	TObjectPtr<UTextBlock> TimerText;

	UPROPERTY()
	TObjectPtr<AAssign05GameState> CachedGameState;

	TSharedPtr<STextBlock> ScoreSlateText;
	TSharedPtr<STextBlock> TimerSlateText;
};
