// Copyright Epic Games, Inc. All Rights Reserved.

#include "Assign05GameMode.h"

#include "Assign05Character.h"
#include "Assign05GameState.h"
#include "Kismet/GameplayStatics.h"
#include "PickupItemBase.h"
#include "SpawnVolume.h"

AAssign05GameMode::AAssign05GameMode()
{
	DefaultPawnClass = AAssign05Character::StaticClass();
	GameStateClass = AAssign05GameState::StaticClass();
	BuildDefaultWaveTable();
}

void AAssign05GameMode::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStartGameFlow)
	{
		StartGameFlow();
	}
}

void AAssign05GameMode::StartGameFlow()
{
	if (LevelConfigs.Num() == 0)
	{
		BuildDefaultWaveTable();
	}

	CurrentLevelIndex = 0;
	CurrentWaveIndex = 0;
	StartCurrentWave();
}

void AAssign05GameMode::StartCurrentWave()
{
	if (!LevelConfigs.IsValidIndex(CurrentLevelIndex) || !LevelConfigs[CurrentLevelIndex].Waves.IsValidIndex(CurrentWaveIndex))
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(WaveTimerHandle);
	GetWorldTimerManager().ClearTimer(NextWaveTimerHandle);

	const FAssign05LevelWaveConfig& LevelConfig = LevelConfigs[CurrentLevelIndex];
	const FAssign05WaveConfig& WaveConfig = LevelConfig.Waves[CurrentWaveIndex];
	CurrentWaveCollectedItems = 0;
	ClearExistingWavePickups();

	if (AAssign05GameState* AssignGameState = GetGameState<AAssign05GameState>())
	{
		AssignGameState->SetWaveState(LevelConfig.LevelNumber, WaveConfig.WaveNumber, WaveConfig.TimeLimit, WaveConfig.RequiredPickupCount);
		const FText Message = WaveConfig.StartMessage.IsEmpty() ? BuildDefaultWaveMessage(WaveConfig) : WaveConfig.StartMessage;
		AssignGameState->BroadcastWaveMessage(Message);
	}

	const FString DebugMessage = FString::Printf(TEXT("Level %d - Wave %d Start!"), LevelConfig.LevelNumber, WaveConfig.WaveNumber);
	UE_LOG(LogTemp, Log, TEXT("%s"), *DebugMessage);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Cyan, DebugMessage);
	}

	SpawnItemsForCurrentWave(WaveConfig);
	GetWorldTimerManager().SetTimer(WaveTimerHandle, this, &AAssign05GameMode::TickWaveTimer, 1.0f, true);
}

void AAssign05GameMode::EndCurrentWave()
{
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);
	AdvanceToNextWaveOrLevel();
}

void AAssign05GameMode::NotifyPickupCollected(int32 ScoreValue)
{
	++CurrentWaveCollectedItems;

	if (AAssign05GameState* AssignGameState = GetGameState<AAssign05GameState>())
	{
		AssignGameState->AddScore(ScoreValue);
		AssignGameState->AddPickupProgress(1);

		if (AssignGameState->GetTargetPickups() > 0 && AssignGameState->GetPickupProgress() >= AssignGameState->GetTargetPickups())
		{
			EndCurrentWave();
		}
	}
}

void AAssign05GameMode::BuildDefaultWaveTable()
{
	LevelConfigs.Empty();

	for (int32 LevelNumber = 1; LevelNumber <= 3; ++LevelNumber)
	{
		FAssign05LevelWaveConfig LevelConfig;
		LevelConfig.LevelNumber = LevelNumber;

		for (int32 WaveNumber = 1; WaveNumber <= 3; ++WaveNumber)
		{
			FAssign05WaveConfig WaveConfig;
			WaveConfig.WaveNumber = WaveNumber;
			WaveConfig.TimeLimit = 35.0f - (WaveNumber * 3.0f) - (LevelNumber * 2.0f);
			WaveConfig.ItemSpawnCount = 4 + WaveNumber + LevelNumber;
			WaveConfig.RequiredPickupCount = FMath::Max(2, WaveConfig.ItemSpawnCount - 2);
			LevelConfig.Waves.Add(WaveConfig);
		}

		LevelConfigs.Add(LevelConfig);
	}
}

void AAssign05GameMode::TickWaveTimer()
{
	AAssign05GameState* AssignGameState = GetGameState<AAssign05GameState>();
	if (AssignGameState == nullptr)
	{
		return;
	}

	const float NewTimeRemaining = AssignGameState->GetTimeRemaining() - 1.0f;
	AssignGameState->SetTimeRemaining(NewTimeRemaining);

	if (NewTimeRemaining <= 0.0f)
	{
		EndCurrentWave();
	}
}

void AAssign05GameMode::AdvanceToNextWaveOrLevel()
{
	++CurrentWaveIndex;

	if (LevelConfigs.IsValidIndex(CurrentLevelIndex) && LevelConfigs[CurrentLevelIndex].Waves.IsValidIndex(CurrentWaveIndex))
	{
		GetWorldTimerManager().SetTimer(NextWaveTimerHandle, this, &AAssign05GameMode::StartCurrentWave, TimeBetweenWaves, false);
		return;
	}

	CurrentWaveIndex = 0;
	++CurrentLevelIndex;

	if (!LevelConfigs.IsValidIndex(CurrentLevelIndex))
	{
		if (AAssign05GameState* AssignGameState = GetGameState<AAssign05GameState>())
		{
			AssignGameState->BroadcastWaveMessage(FText::FromString(TEXT("All levels cleared!")));
		}
		return;
	}

	const FAssign05LevelWaveConfig& NextLevelConfig = LevelConfigs[CurrentLevelIndex];
	if (bTravelToOptionalMapNames && !NextLevelConfig.OptionalMapName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, NextLevelConfig.OptionalMapName);
		return;
	}

	GetWorldTimerManager().SetTimer(NextWaveTimerHandle, this, &AAssign05GameMode::StartCurrentWave, TimeBetweenWaves, false);
}

void AAssign05GameMode::SpawnItemsForCurrentWave(const FAssign05WaveConfig& WaveConfig)
{
	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(this, ASpawnVolume::StaticClass(), FoundVolumes);

	if (FoundVolumes.Num() == 0)
	{
		return;
	}

	int32 RemainingSpawns = WaveConfig.ItemSpawnCount;
	for (int32 Index = 0; Index < FoundVolumes.Num() && RemainingSpawns > 0; ++Index)
	{
		ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[Index]);
		if (SpawnVolume == nullptr)
		{
			continue;
		}

		const int32 VolumesLeft = FoundVolumes.Num() - Index;
		const int32 SpawnCountForVolume = FMath::CeilToInt(static_cast<float>(RemainingSpawns) / static_cast<float>(VolumesLeft));
		SpawnVolume->SpawnWaveItems(SpawnCountForVolume);
		RemainingSpawns -= SpawnCountForVolume;
	}
}

void AAssign05GameMode::ClearExistingWavePickups()
{
	TArray<AActor*> ExistingPickups;
	UGameplayStatics::GetAllActorsOfClass(this, APickupItemBase::StaticClass(), ExistingPickups);

	for (AActor* Pickup : ExistingPickups)
	{
		if (IsValid(Pickup))
		{
			Pickup->Destroy();
		}
	}
}

FText AAssign05GameMode::BuildDefaultWaveMessage(const FAssign05WaveConfig& WaveConfig) const
{
	const int32 LevelNumber = LevelConfigs.IsValidIndex(CurrentLevelIndex) ? LevelConfigs[CurrentLevelIndex].LevelNumber : CurrentLevelIndex + 1;
	return FText::FromString(FString::Printf(TEXT("Level %d - Wave %d Start!"), LevelNumber, WaveConfig.WaveNumber));
}
