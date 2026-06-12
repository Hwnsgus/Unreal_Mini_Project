// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Assign05WaveTypes.generated.h"

USTRUCT(BlueprintType)
struct FAssign05WaveConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 WaveNumber = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (ClampMin = "1.0"))
	float TimeLimit = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (ClampMin = "0"))
	int32 ItemSpawnCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (ClampMin = "0"))
	int32 RequiredPickupCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	FText StartMessage;
};

USTRUCT(BlueprintType)
struct FAssign05LevelWaveConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	int32 LevelNumber = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	FName OptionalMapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	TArray<FAssign05WaveConfig> Waves;
};
