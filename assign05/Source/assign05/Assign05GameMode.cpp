// Copyright Epic Games, Inc. All Rights Reserved.

#include "Assign05GameMode.h"

#include "Assign05Character.h"
#include "Assign05GameInstance.h"
#include "Assign05GameState.h"
#include "Assign05StageTransitionWidget.h"
#include "Kismet/GameplayStatics.h"
#include "PickupItemBase.h"
#include "SpawnVolume.h"

AAssign05GameMode::AAssign05GameMode()
{
	DefaultPawnClass = AAssign05Character::StaticClass();
	GameStateClass = AAssign05GameState::StaticClass();
	StageTransitionWidgetClass = UAssign05StageTransitionWidget::StaticClass();
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
	EnsureDefaultMapAssignments();

	CurrentLevelIndex = 0;
	CurrentWaveIndex = 0;
	CarriedScore = 0;

	if (UAssign05GameInstance* AssignGameInstance = GetGameInstance<UAssign05GameInstance>())
	{
		int32 SavedLevelIndex = 0;
		int32 SavedWaveIndex = 0;
		int32 SavedScore = 0;
		if (AssignGameInstance->ConsumeWaveProgress(SavedLevelIndex, SavedWaveIndex, SavedScore))
		{
			CurrentLevelIndex = SavedLevelIndex;
			CurrentWaveIndex = SavedWaveIndex;
			CarriedScore = SavedScore;
		}
	}

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
	ShowStageTransitionUI(LevelConfig, WaveConfig);

	if (AAssign05GameState* AssignGameState = GetGameState<AAssign05GameState>())
	{
		AssignGameState->SetWaveState(LevelConfig.LevelNumber, WaveConfig.WaveNumber, WaveConfig.TimeLimit, WaveConfig.RequiredPickupCount);
		AssignGameState->SetScore(CarriedScore);
		const FText Message = WaveConfig.StartMessage.IsEmpty() ? BuildDefaultWaveMessage(WaveConfig) : WaveConfig.StartMessage;
		AssignGameState->BroadcastWaveMessage(Message);
	}

	const FString DebugMessage = FString::Printf(TEXT("Level %d - Wave %d Start!"), LevelConfig.LevelNumber, WaveConfig.WaveNumber);
	UE_LOG(LogTemp, Log, TEXT("%s"), *DebugMessage);

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
		CarriedScore = AssignGameState->GetScore();
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
		if (LevelNumber == 1)
		{
			LevelConfig.OptionalMap = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Assign05/Maps/Round1.Round1")));
		}
		else if (LevelNumber == 2)
		{
			LevelConfig.OptionalMap = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Assign05/Maps/Round2.Round2")));
		}
		else if (LevelNumber == 3)
		{
			LevelConfig.OptionalMap = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Assign05/Maps/Round3.Round3")));
		}

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

void AAssign05GameMode::EnsureDefaultMapAssignments()
{
	for (int32 Index = 0; Index < LevelConfigs.Num(); ++Index)
	{
		if (!LevelConfigs[Index].OptionalMap.IsNull())
		{
			continue;
		}

		if (Index == 0)
		{
			LevelConfigs[Index].OptionalMap = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Assign05/Maps/Round1.Round1")));
		}
		else if (Index == 1)
		{
			LevelConfigs[Index].OptionalMap = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Assign05/Maps/Round2.Round2")));
		}
		else if (Index == 2)
		{
			LevelConfigs[Index].OptionalMap = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Assign05/Maps/Round3.Round3")));
		}
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
		if (TryTravelToWaveMap(CurrentLevelIndex, CurrentWaveIndex))
		{
			return;
		}

		GetWorldTimerManager().SetTimer(NextWaveTimerHandle, this, &AAssign05GameMode::StartCurrentWave, TimeBetweenWaves, false);
		return;
	}

	CurrentWaveIndex = 0;
	++CurrentLevelIndex;

	if (!LevelConfigs.IsValidIndex(CurrentLevelIndex))
	{
		ClearExistingWavePickups();
		if (AAssign05GameState* AssignGameState = GetGameState<AAssign05GameState>())
		{
			AssignGameState->BroadcastWaveMessage(FText::FromString(TEXT("All levels cleared!")));
		}
		ShowGameClearUI();
		return;
	}

	if (TryTravelToWaveMap(CurrentLevelIndex, CurrentWaveIndex))
	{
		return;
	}

	GetWorldTimerManager().SetTimer(NextWaveTimerHandle, this, &AAssign05GameMode::StartCurrentWave, TimeBetweenWaves, false);
}

bool AAssign05GameMode::TryTravelToWaveMap(int32 NextLevelIndex, int32 NextWaveIndex)
{
	if (!bTravelToOptionalMapNames)
	{
		return false;
	}

	const FName TravelMapName = GetTravelMapName(NextLevelIndex, NextWaveIndex);
	if (TravelMapName.IsNone())
	{
		return false;
	}

	if (UAssign05GameInstance* AssignGameInstance = GetGameInstance<UAssign05GameInstance>())
	{
		AssignGameInstance->SaveWaveProgress(NextLevelIndex, NextWaveIndex, CarriedScore);
	}

	UGameplayStatics::OpenLevel(this, TravelMapName);
	return true;
}

FName AAssign05GameMode::GetTravelMapName(int32 NextLevelIndex, int32 NextWaveIndex) const
{
	if (!LevelConfigs.IsValidIndex(NextLevelIndex) || !LevelConfigs[NextLevelIndex].Waves.IsValidIndex(NextWaveIndex))
	{
		return NAME_None;
	}

	const FAssign05LevelWaveConfig& LevelConfig = LevelConfigs[NextLevelIndex];
	const FAssign05WaveConfig& WaveConfig = LevelConfig.Waves[NextWaveIndex];

	TSoftObjectPtr<UWorld> MapToLoad = WaveConfig.OptionalMap;
	if (MapToLoad.IsNull() && NextWaveIndex == 0)
	{
		MapToLoad = LevelConfig.OptionalMap;
	}

	if (MapToLoad.IsNull())
	{
		return NAME_None;
	}

	return FName(*MapToLoad.ToSoftObjectPath().GetLongPackageName());
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

void AAssign05GameMode::ShowStageTransitionUI(const FAssign05LevelWaveConfig& LevelConfig, const FAssign05WaveConfig& WaveConfig)
{
	if (StageTransitionWidgetClass == nullptr)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController == nullptr)
	{
		return;
	}

	if (ActiveStageTransitionWidget)
	{
		ActiveStageTransitionWidget->RemoveFromParent();
	}

	ActiveStageTransitionWidget = CreateWidget<UAssign05StageTransitionWidget>(PlayerController, StageTransitionWidgetClass);
	if (ActiveStageTransitionWidget == nullptr)
	{
		return;
	}

	ActiveStageTransitionWidget->AddToViewport(80);
	ActiveStageTransitionWidget->ShowStageTransition(LevelConfig.LevelNumber, WaveConfig.WaveNumber, WaveConfig.TimeLimit, WaveConfig.ItemSpawnCount, WaveConfig.RequiredPickupCount);
}

void AAssign05GameMode::ShowGameClearUI()
{
	if (StageTransitionWidgetClass == nullptr)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController == nullptr)
	{
		return;
	}

	if (ActiveStageTransitionWidget)
	{
		ActiveStageTransitionWidget->RemoveFromParent();
	}

	ActiveStageTransitionWidget = CreateWidget<UAssign05StageTransitionWidget>(PlayerController, StageTransitionWidgetClass);
	if (ActiveStageTransitionWidget == nullptr)
	{
		return;
	}

	int32 FinalScore = 0;
	if (const AAssign05GameState* AssignGameState = GetGameState<AAssign05GameState>())
	{
		FinalScore = AssignGameState->GetScore();
	}

	ActiveStageTransitionWidget->AddToViewport(90);
	ActiveStageTransitionWidget->ShowGameClear(FinalScore);
}

FText AAssign05GameMode::BuildDefaultWaveMessage(const FAssign05WaveConfig& WaveConfig) const
{
	const int32 LevelNumber = LevelConfigs.IsValidIndex(CurrentLevelIndex) ? LevelConfigs[CurrentLevelIndex].LevelNumber : CurrentLevelIndex + 1;
	return FText::FromString(FString::Printf(TEXT("Level %d - Wave %d Start!"), LevelNumber, WaveConfig.WaveNumber));
}
