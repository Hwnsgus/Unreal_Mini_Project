// Copyright Epic Games, Inc. All Rights Reserved.

#include "Assign05GameMode.h"

#include "Assign05Character.h"
#include "Assign05GameInstance.h"
#include "Assign05GameState.h"
#include "Assign05HUDWidget.h"
#include "Assign05StageTransitionWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "PickupItemBase.h"
#include "SpawnVolume.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
bool SetEndGameTextByNames(UUserWidget* EndGameWidget, const TArray<FName>& CandidateNames, const FText& Text)
{
	if (EndGameWidget == nullptr)
	{
		return false;
	}

	for (const FName& WidgetName : CandidateNames)
	{
		if (UTextBlock* TextBlock = Cast<UTextBlock>(EndGameWidget->GetWidgetFromName(WidgetName)))
		{
			TextBlock->SetText(Text);
			return true;
		}

		if (UEditableTextBox* TextBox = Cast<UEditableTextBox>(EndGameWidget->GetWidgetFromName(WidgetName)))
		{
			TextBox->SetText(Text);
			TextBox->SetIsReadOnly(true);
			return true;
		}
	}

	return false;
}

bool TryCallEndGameAnimationFunction(UUserWidget* EndGameWidget)
{
	if (EndGameWidget == nullptr)
	{
		return false;
	}

	static const FName FunctionNames[] =
	{
		TEXT("PlayResultTextAnimation"),
		TEXT("PlayResultAnimation"),
		TEXT("PlayEndGameAnimation"),
		TEXT("PlayGameOverAnimation"),
		TEXT("PlayBlinkAnimation"),
		TEXT("PlayResultTextBlinkAnimation"),
		TEXT("StartResultTextAnimation"),
		TEXT("StartResultAnimation"),
		TEXT("StartBlinkAnimation"),
		TEXT("ResultTextAnimation"),
		TEXT("ResultTextBlink"),
		TEXT("BlinkResultText")
	};

	for (const FName& FunctionName : FunctionNames)
	{
		if (UFunction* AnimationFunction = EndGameWidget->FindFunction(FunctionName))
		{
			if (AnimationFunction->NumParms == 0)
			{
				EndGameWidget->ProcessEvent(AnimationFunction, nullptr);
				return true;
			}
		}
	}

	return false;
}

void PlayEndGameAnimationByAsset(UUserWidget* EndGameWidget)
{
	if (EndGameWidget == nullptr)
	{
		return;
	}

	UWidgetBlueprintGeneratedClass* WidgetClass = Cast<UWidgetBlueprintGeneratedClass>(EndGameWidget->GetClass());
	if (WidgetClass == nullptr)
	{
		return;
	}

	UWidgetAnimation* FallbackAnimation = nullptr;
	for (UWidgetAnimation* Animation : WidgetClass->Animations)
	{
		if (Animation == nullptr)
		{
			continue;
		}

		if (FallbackAnimation == nullptr)
		{
			FallbackAnimation = Animation;
		}

		const FString AnimationName = Animation->GetName();
		if (AnimationName.Contains(TEXT("Result")) || AnimationName.Contains(TEXT("Blink")) || AnimationName.Contains(TEXT("Text")))
		{
			EndGameWidget->PlayAnimation(Animation, 0.0f, 0);
			return;
		}
	}

	if (FallbackAnimation)
	{
		EndGameWidget->PlayAnimation(FallbackAnimation, 0.0f, 0);
	}
}

void PlayEndGamePresentation(UUserWidget* EndGameWidget)
{
	if (!TryCallEndGameAnimationFunction(EndGameWidget))
	{
		PlayEndGameAnimationByAsset(EndGameWidget);
	}
}
}

AAssign05GameMode::AAssign05GameMode()
{
	DefaultPawnClass = AAssign05Character::StaticClass();
	GameStateClass = AAssign05GameState::StaticClass();
	HUDWidgetClass = UAssign05HUDWidget::StaticClass();
	StageTransitionWidgetClass = UAssign05StageTransitionWidget::StaticClass();
	RestartMap = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Assign05/Maps/Round1.Round1")));

	static ConstructorHelpers::FClassFinder<UAssign05HUDWidget> HUDWidgetFinder(TEXT("/Game/Assign05/UI/WBP_HUD"));
	if (HUDWidgetFinder.Succeeded())
	{
		HUDWidgetClass = HUDWidgetFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> EndGameWidgetFinder(TEXT("/Game/Assign05/UI/WBP_EndGame"));
	if (EndGameWidgetFinder.Succeeded())
	{
		EndGameWidgetClass = EndGameWidgetFinder.Class;
	}

	BuildDefaultWaveTable();
}

void AAssign05GameMode::BeginPlay()
{
	Super::BeginPlay();

	ShowHUD();

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
	NormalizeLevelConfigsForSingleWaveRounds();

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
			CurrentLevelIndex = FMath::Clamp(SavedLevelIndex, 0, FMath::Max(0, LevelConfigs.Num() - 1));
			CurrentWaveIndex = 0;
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

void AAssign05GameMode::RestartGameFromFirstRound()
{
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);
	GetWorldTimerManager().ClearTimer(NextWaveTimerHandle);
	ClearExistingWavePickups();

	if (UAssign05GameInstance* AssignGameInstance = GetGameInstance<UAssign05GameInstance>())
	{
		AssignGameInstance->ResetWaveProgress();
	}

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputMode(FInputModeGameOnly());
	}

	FName MapToLoad = NAME_None;
	if (!RestartMap.IsNull())
	{
		MapToLoad = FName(*RestartMap.ToSoftObjectPath().GetLongPackageName());
	}

	if (MapToLoad.IsNone())
	{
		EnsureDefaultMapAssignments();
		if (LevelConfigs.IsValidIndex(0) && !LevelConfigs[0].OptionalMap.IsNull())
		{
			MapToLoad = FName(*LevelConfigs[0].OptionalMap.ToSoftObjectPath().GetLongPackageName());
		}
	}

	if (MapToLoad.IsNone())
	{
		MapToLoad = FName(TEXT("/Game/Assign05/Maps/Round1"));
	}

	UGameplayStatics::OpenLevel(this, MapToLoad);
}

void AAssign05GameMode::TriggerGameOver()
{
	TriggerGameOverWithMessage(FText::FromString(TEXT("GAME OVER")));
}

void AAssign05GameMode::TriggerGameOverWithMessage(const FText& GameOverMessage)
{
	ShowGameOverUI(GameOverMessage);
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

		FAssign05WaveConfig WaveConfig;
		WaveConfig.WaveNumber = 1;
		WaveConfig.TimeLimit = 30.0f;
		WaveConfig.ItemSpawnCount = 5 + LevelNumber;
		WaveConfig.RequiredPickupCount = FMath::Max(2, WaveConfig.ItemSpawnCount - 2);
		LevelConfig.Waves.Add(WaveConfig);

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

void AAssign05GameMode::NormalizeLevelConfigsForSingleWaveRounds()
{
	for (int32 Index = 0; Index < LevelConfigs.Num(); ++Index)
	{
		FAssign05LevelWaveConfig& LevelConfig = LevelConfigs[Index];
		LevelConfig.LevelNumber = Index + 1;

		if (LevelConfig.Waves.Num() == 0)
		{
			FAssign05WaveConfig DefaultWave;
			DefaultWave.WaveNumber = 1;
			DefaultWave.TimeLimit = 30.0f;
			DefaultWave.ItemSpawnCount = 6 + Index;
			DefaultWave.RequiredPickupCount = FMath::Max(2, DefaultWave.ItemSpawnCount - 2);
			LevelConfig.Waves.Add(DefaultWave);
		}
		else if (LevelConfig.Waves.Num() > 1)
		{
			LevelConfig.Waves.SetNum(1);
		}

		FAssign05WaveConfig& WaveConfig = LevelConfig.Waves[0];
		WaveConfig.WaveNumber = 1;
		WaveConfig.TimeLimit = 30.0f;
		WaveConfig.OptionalMap.Reset();
		WaveConfig.ItemSpawnCount = FMath::Max(1, WaveConfig.ItemSpawnCount);
		WaveConfig.RequiredPickupCount = FMath::Clamp(WaveConfig.RequiredPickupCount, 1, WaveConfig.ItemSpawnCount);
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
		ShowGameOverUI(FText::FromString(TEXT("TIME OVER")));
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

	auto SpawnAcrossVolumes = [&FoundVolumes](int32 SpawnCount, bool bRequiredPickup)
	{
		int32 RemainingSpawns = SpawnCount;
		for (int32 Index = 0; Index < FoundVolumes.Num() && RemainingSpawns > 0; ++Index)
		{
			ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[Index]);
			if (SpawnVolume == nullptr)
			{
				continue;
			}

			const int32 VolumesLeft = FoundVolumes.Num() - Index;
			const int32 SpawnCountForVolume = FMath::CeilToInt(static_cast<float>(RemainingSpawns) / static_cast<float>(VolumesLeft));
			if (bRequiredPickup)
			{
				SpawnVolume->SpawnRequiredPickupItems(SpawnCountForVolume);
			}
			else
			{
				SpawnVolume->SpawnWaveItems(SpawnCountForVolume);
			}
			RemainingSpawns -= SpawnCountForVolume;
		}
	};

	const int32 RequiredPickupSpawnCount = FMath::Clamp(WaveConfig.RequiredPickupCount, 0, WaveConfig.ItemSpawnCount);
	const int32 BonusSpawnCount = FMath::Max(0, WaveConfig.ItemSpawnCount - RequiredPickupSpawnCount);

	SpawnAcrossVolumes(RequiredPickupSpawnCount, true);
	SpawnAcrossVolumes(BonusSpawnCount, false);
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

void AAssign05GameMode::ShowHUD()
{
	if (HUDWidgetClass == nullptr)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController == nullptr)
	{
		return;
	}

	if (ActiveHUDWidget)
	{
		ActiveHUDWidget->RemoveFromParent();
	}

	ActiveHUDWidget = CreateWidget<UAssign05HUDWidget>(PlayerController, HUDWidgetClass);
	if (ActiveHUDWidget == nullptr)
	{
		return;
	}

	ActiveHUDWidget->AddToViewport(10);
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

void AAssign05GameMode::ShowGameOverUI(const FText& GameOverMessage)
{
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);
	GetWorldTimerManager().ClearTimer(NextWaveTimerHandle);
	ClearExistingWavePickups();

	if (AAssign05GameState* AssignGameState = GetGameState<AAssign05GameState>())
	{
		AssignGameState->SetTimeRemaining(0.0f);
		AssignGameState->BroadcastWaveMessage(GameOverMessage);
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

	if (ActiveEndGameWidget)
	{
		ActiveEndGameWidget->RemoveFromParent();
	}

	if (EndGameWidgetClass)
	{
		ActiveEndGameWidget = CreateWidget<UUserWidget>(PlayerController, EndGameWidgetClass);
		if (ActiveEndGameWidget == nullptr)
		{
			return;
		}

		ActiveEndGameWidget->AddToViewport(100);
		ApplyEndGameWidgetText(ActiveEndGameWidget, GameOverMessage);
		PlayEndGamePresentation(ActiveEndGameWidget);
		BindEndGameRetryButton(ActiveEndGameWidget);
	}

	PlayerController->bShowMouseCursor = true;
	PlayerController->SetInputMode(FInputModeUIOnly());
}

void AAssign05GameMode::ShowGameClearUI()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController == nullptr)
	{
		return;
	}

	if (ActiveStageTransitionWidget)
	{
		ActiveStageTransitionWidget->RemoveFromParent();
	}

	if (ActiveEndGameWidget)
	{
		ActiveEndGameWidget->RemoveFromParent();
	}

	if (EndGameWidgetClass)
	{
		ActiveEndGameWidget = CreateWidget<UUserWidget>(PlayerController, EndGameWidgetClass);
		if (ActiveEndGameWidget == nullptr)
		{
			return;
		}

		ActiveEndGameWidget->AddToViewport(100);
		ApplyEndGameWidgetText(ActiveEndGameWidget, FText::FromString(TEXT("STAGE CLEAR")));
		PlayEndGamePresentation(ActiveEndGameWidget);
		BindEndGameRetryButton(ActiveEndGameWidget);

		PlayerController->bShowMouseCursor = true;
		PlayerController->SetInputMode(FInputModeUIOnly());
		return;
	}

	if (StageTransitionWidgetClass == nullptr)
	{
		return;
	}

	int32 FinalScore = 0;
	if (const AAssign05GameState* AssignGameState = GetGameState<AAssign05GameState>())
	{
		FinalScore = AssignGameState->GetScore();
	}

	ActiveStageTransitionWidget = CreateWidget<UAssign05StageTransitionWidget>(PlayerController, StageTransitionWidgetClass);
	if (ActiveStageTransitionWidget == nullptr)
	{
		return;
	}

	ActiveStageTransitionWidget->AddToViewport(90);
	ActiveStageTransitionWidget->ShowGameClear(FinalScore);
}

void AAssign05GameMode::ApplyEndGameWidgetText(UUserWidget* EndGameWidget, const FText& ResultText)
{
	const int32 FinalScore = GetCurrentScore();
	const FText ScoreText = FText::FromString(FString::Printf(TEXT("Total Score: %d"), FinalScore));

	static const TArray<FName> ResultTextNames =
	{
		TEXT("ResultText"),
		TEXT("TitleText"),
		TEXT("HeaderText"),
		TEXT("GameOverText"),
		TEXT("ClearText")
	};

	static const TArray<FName> ScoreTextNames =
	{
		TEXT("FinalScoreText"),
		TEXT("TotalScoreText"),
		TEXT("ScoreText"),
		TEXT("ResultScoreText")
	};

	SetEndGameTextByNames(EndGameWidget, ResultTextNames, ResultText);
	if (!SetEndGameTextByNames(EndGameWidget, ScoreTextNames, ScoreText))
	{
		UE_LOG(LogTemp, Warning, TEXT("WBP_EndGame score text was not found. Name a TextBlock 'FinalScoreText' or 'TotalScoreText'."));
	}
}

void AAssign05GameMode::BindEndGameRetryButton(UUserWidget* EndGameWidget)
{
	UButton* RetryButton = FindRetryButton(EndGameWidget);
	if (RetryButton == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("WBP_EndGame retry button was not found. Name it 'RetryButton' or 'retry'."));
		return;
	}

	RetryButton->OnClicked.RemoveDynamic(this, &AAssign05GameMode::HandleRetryClicked);
	RetryButton->OnClicked.AddDynamic(this, &AAssign05GameMode::HandleRetryClicked);
}

UButton* AAssign05GameMode::FindRetryButton(UUserWidget* EndGameWidget) const
{
	if (EndGameWidget == nullptr)
	{
		return nullptr;
	}

	static const FName RetryButtonNames[] =
	{
		TEXT("RetryButton"),
		TEXT("retry"),
		TEXT("Retry"),
		TEXT("Button"),
		TEXT("Button_0")
	};

	for (const FName ButtonName : RetryButtonNames)
	{
		if (UButton* RetryButton = Cast<UButton>(EndGameWidget->GetWidgetFromName(ButtonName)))
		{
			return RetryButton;
		}
	}

	return nullptr;
}

int32 AAssign05GameMode::GetCurrentScore() const
{
	if (const AAssign05GameState* AssignGameState = GetGameState<AAssign05GameState>())
	{
		return AssignGameState->GetScore();
	}

	return CarriedScore;
}

void AAssign05GameMode::HandleRetryClicked()
{
	RestartGameFromFirstRound();
}

FText AAssign05GameMode::BuildDefaultWaveMessage(const FAssign05WaveConfig& WaveConfig) const
{
	const int32 LevelNumber = LevelConfigs.IsValidIndex(CurrentLevelIndex) ? LevelConfigs[CurrentLevelIndex].LevelNumber : CurrentLevelIndex + 1;
	return FText::FromString(FString::Printf(TEXT("Level %d - Wave %d Start!"), LevelNumber, WaveConfig.WaveNumber));
}
