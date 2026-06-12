// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Assign05WaveTypes.h"
#include "GameFramework/GameModeBase.h"
#include "Assign05GameMode.generated.h"

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

protected:
	void BuildDefaultWaveTable();
	void TickWaveTimer();
	void AdvanceToNextWaveOrLevel();
	void SpawnItemsForCurrentWave(const FAssign05WaveConfig& WaveConfig);
	void ClearExistingWavePickups();
	FText BuildDefaultWaveMessage(const FAssign05WaveConfig& WaveConfig) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<FAssign05LevelWaveConfig> LevelConfigs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float TimeBetweenWaves = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	bool bAutoStartGameFlow = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	bool bTravelToOptionalMapNames = false;

	UPROPERTY(BlueprintReadOnly, Category = "Wave")
	int32 CurrentLevelIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Wave")
	int32 CurrentWaveIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Wave")
	int32 CurrentWaveCollectedItems = 0;

private:
	FTimerHandle WaveTimerHandle;
	FTimerHandle NextWaveTimerHandle;
};
