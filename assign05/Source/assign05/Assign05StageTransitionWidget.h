// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Assign05StageTransitionWidget.generated.h"

class UBorder;
class UEditableTextBox;
class UProgressBar;
class UTextBlock;
class UVerticalBox;

UCLASS()
class ASSIGN05_API UAssign05StageTransitionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void ShowStageTransition(int32 LevelNumber, int32 WaveNumber, float TimeLimit, int32 SpawnCount, int32 RequiredPickupCount);

	UFUNCTION(BlueprintCallable, Category = "Stage")
	void ShowGameClear(int32 FinalScore);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void BuildWidgetTree();
	void CacheDesignerWidgets();
	void SetDisplayText(UTextBlock* TextBlock, UEditableTextBox* TextBox, const FText& Text) const;
	void SetDisplayTextColor(UTextBlock* TextBlock, UEditableTextBox* TextBox, const FLinearColor& Color) const;
	FLinearColor GetStageColor(int32 LevelNumber) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	float DisplayDuration = 2.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	float FadeDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	float ClearDisplayDuration = 6.0f;

private:
	UPROPERTY()
	TObjectPtr<UBorder> CardBorder;

	UPROPERTY()
	TObjectPtr<UTextBlock> HeaderText;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> HeaderTextBox;

	UPROPERTY()
	TObjectPtr<UTextBlock> StageText;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> StageTextBox;

	UPROPERTY()
	TObjectPtr<UTextBlock> DetailText;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> DetailTextBox;

	UPROPERTY()
	TObjectPtr<UProgressBar> AccentBar;

	float ElapsedTime = 0.0f;
	float CurrentDisplayDuration = 0.0f;
	bool bIsShowing = false;
};
