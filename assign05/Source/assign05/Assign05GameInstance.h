// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Assign05GameInstance.generated.h"

UCLASS()
class ASSIGN05_API UAssign05GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void SaveWaveProgress(int32 LevelIndex, int32 WaveIndex, int32 Score);

	UFUNCTION(BlueprintCallable, Category = "Wave")
	bool ConsumeWaveProgress(int32& OutLevelIndex, int32& OutWaveIndex, int32& OutScore);

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void ResetWaveProgress();

	UFUNCTION(BlueprintPure, Category = "Wave")
	int32 GetSavedScore() const { return SavedScore; }

private:
	UPROPERTY()
	int32 SavedLevelIndex = 0;

	UPROPERTY()
	int32 SavedWaveIndex = 0;

	UPROPERTY()
	int32 SavedScore = 0;

	UPROPERTY()
	bool bHasSavedProgress = false;
};
