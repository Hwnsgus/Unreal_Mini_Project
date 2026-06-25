// Copyright Epic Games, Inc. All Rights Reserved.

#include "Assign05GameInstance.h"

void UAssign05GameInstance::SaveWaveProgress(int32 LevelIndex, int32 WaveIndex, int32 Score)
{
	SavedLevelIndex = LevelIndex;
	SavedWaveIndex = WaveIndex;
	SavedScore = Score;
	bHasSavedProgress = true;
}

bool UAssign05GameInstance::ConsumeWaveProgress(int32& OutLevelIndex, int32& OutWaveIndex, int32& OutScore)
{
	if (!bHasSavedProgress)
	{
		return false;
	}

	OutLevelIndex = SavedLevelIndex;
	OutWaveIndex = SavedWaveIndex;
	OutScore = SavedScore;
	bHasSavedProgress = false;
	return true;
}

void UAssign05GameInstance::ResetWaveProgress()
{
	SavedLevelIndex = 0;
	SavedWaveIndex = 0;
	SavedScore = 0;
	bHasSavedProgress = false;
}
