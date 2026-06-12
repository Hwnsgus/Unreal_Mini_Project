// Copyright Epic Games, Inc. All Rights Reserved.

#include "Assign05GameState.h"

void AAssign05GameState::SetWaveState(int32 NewLevel, int32 NewWave, float NewTimeRemaining, int32 NewTargetPickups)
{
	CurrentLevel = NewLevel;
	CurrentWave = NewWave;
	TimeRemaining = NewTimeRemaining;
	TargetPickups = NewTargetPickups;
	PickupProgress = 0;

	OnWaveStateChanged.Broadcast();
}

void AAssign05GameState::SetTimeRemaining(float NewTimeRemaining)
{
	TimeRemaining = FMath::Max(0.0f, NewTimeRemaining);
	OnWaveStateChanged.Broadcast();
}

void AAssign05GameState::AddScore(int32 Amount)
{
	Score += Amount;
	OnWaveStateChanged.Broadcast();
}

void AAssign05GameState::AddPickupProgress(int32 Amount)
{
	PickupProgress += Amount;
	OnWaveStateChanged.Broadcast();
}

void AAssign05GameState::BroadcastWaveMessage(const FText& Message)
{
	OnWaveMessage.Broadcast(Message);
}
