// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Assign05GameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWaveStateChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveMessage, const FText&, Message);

UCLASS()
class ASSIGN05_API AAssign05GameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void SetWaveState(int32 NewLevel, int32 NewWave, float NewTimeRemaining, int32 NewTargetPickups);

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void SetTimeRemaining(float NewTimeRemaining);

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void AddScore(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void SetScore(int32 NewScore);

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void AddPickupProgress(int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void BroadcastWaveMessage(const FText& Message);

	UFUNCTION(BlueprintPure, Category = "Wave")
	int32 GetCurrentLevel() const { return CurrentLevel; }

	UFUNCTION(BlueprintPure, Category = "Wave")
	int32 GetCurrentWave() const { return CurrentWave; }

	UFUNCTION(BlueprintPure, Category = "Wave")
	float GetTimeRemaining() const { return TimeRemaining; }

	UFUNCTION(BlueprintPure, Category = "Wave")
	int32 GetScore() const { return Score; }

	UFUNCTION(BlueprintPure, Category = "Wave")
	int32 GetPickupProgress() const { return PickupProgress; }

	UFUNCTION(BlueprintPure, Category = "Wave")
	int32 GetTargetPickups() const { return TargetPickups; }

public:
	UPROPERTY(BlueprintAssignable, Category = "Wave")
	FOnWaveStateChanged OnWaveStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Wave")
	FOnWaveMessage OnWaveMessage;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Wave")
	int32 CurrentLevel = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Wave")
	int32 CurrentWave = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Wave")
	float TimeRemaining = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Wave")
	int32 Score = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Wave")
	int32 PickupProgress = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Wave")
	int32 TargetPickups = 0;
};
