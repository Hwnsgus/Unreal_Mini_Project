// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Assign05WaveTypes.h"
#include "GameFramework/GameModeBase.h"
#include "Assign05GameMode.generated.h"

class UAssign05StageTransitionWidget;
class UAssign05HUDWidget;
class UButton;
class UUserWidget;

UCLASS()
class ASSIGN05_API AAssign05GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAssign05GameMode();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartGameFlow();

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartCurrentWave();

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void EndCurrentWave();

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void NotifyPickupCollected(int32 ScoreValue);

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void RestartGameFromFirstRound();

protected:
	void BuildDefaultWaveTable();
	void EnsureDefaultMapAssignments();
	void NormalizeLevelConfigsForSingleWaveRounds();
	void TickWaveTimer();
	void AdvanceToNextWaveOrLevel();
	bool TryTravelToWaveMap(int32 NextLevelIndex, int32 NextWaveIndex);
	FName GetTravelMapName(int32 NextLevelIndex, int32 NextWaveIndex) const;
	void SpawnItemsForCurrentWave(const FAssign05WaveConfig& WaveConfig);
	void ClearExistingWavePickups();
	void ShowHUD();
	void ShowStageTransitionUI(const FAssign05LevelWaveConfig& LevelConfig, const FAssign05WaveConfig& WaveConfig);
	void ShowGameOverUI();
	void ShowGameClearUI();
	void BindEndGameRetryButton(UUserWidget* EndGameWidget);
	UButton* FindRetryButton(UUserWidget* EndGameWidget) const;
	FText BuildDefaultWaveMessage(const FAssign05WaveConfig& WaveConfig) const;

	UFUNCTION()
	void HandleRetryClicked();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<FAssign05LevelWaveConfig> LevelConfigs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float TimeBetweenWaves = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	bool bAutoStartGameFlow = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	bool bTravelToOptionalMapNames = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UAssign05StageTransitionWidget> StageTransitionWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UAssign05HUDWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> EndGameWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave|Map")
	TSoftObjectPtr<UWorld> RestartMap;

	UPROPERTY(BlueprintReadOnly, Category = "Wave")
	int32 CurrentLevelIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Wave")
	int32 CurrentWaveIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Wave")
	int32 CurrentWaveCollectedItems = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Wave")
	int32 CarriedScore = 0;

private:
	FTimerHandle WaveTimerHandle;
	FTimerHandle NextWaveTimerHandle;

	UPROPERTY()
	TObjectPtr<UAssign05StageTransitionWidget> ActiveStageTransitionWidget;

	UPROPERTY()
	TObjectPtr<UAssign05HUDWidget> ActiveHUDWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> ActiveEndGameWidget;
};
