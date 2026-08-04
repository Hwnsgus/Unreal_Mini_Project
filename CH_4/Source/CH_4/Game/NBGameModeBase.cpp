#include "Game/NBGameModeBase.h"

#include "EngineUtils.h"
#include "Game/NBGameStateBase.h"
#include "Player/NBPlayerController.h"
#include "Player/NBPlayerState.h"

ANBGameModeBase::ANBGameModeBase()
    : TurnDurationSeconds(30.0f)
    , MinimumPlayersToStart(2)
    , GameResetDelaySeconds(3.0f)
    , NextPlayerNumber(1)
    , bGameInProgress(false)
{
    PlayerControllerClass = ANBPlayerController::StaticClass();
    PlayerStateClass = ANBPlayerState::StaticClass();
    GameStateClass = ANBGameStateBase::StaticClass();

    // This project is UI-driven, so a default Pawn is not required.
    DefaultPawnClass = nullptr;
}

void ANBGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    SecretNumber = GenerateSecretNumber();

    UE_LOG(
        LogTemp,
        Log,
        TEXT("숫자 야구를 시작합니다. 주어진 번호: %s, NetMode: %d"),
        *SecretNumber,
        static_cast<int32>(GetNetMode())
    );
}

void ANBGameModeBase::OnPostLogin(AController* NewPlayer)
{
    Super::OnPostLogin(NewPlayer);

    ANBPlayerController* PlayerController =
        Cast<ANBPlayerController>(NewPlayer);

    if (!IsValid(PlayerController))
    {
        return;
    }

    ANBPlayerState* PlayerState =
        PlayerController->GetPlayerState<ANBPlayerState>();

    if (!IsValid(PlayerState))
    {
        return;
    }

    const int32 AssignedPlayerNumber = NextPlayerNumber++;
    PlayerState->SetPlayerNumber(AssignedPlayerNumber);

    PlayerController->ClientRPCReceiveMessage(
        FString::Printf(
            TEXT("당신은 %d 플레이어입니다. 1부터 9까지 고유 숫자 3개를 입력하세요."),
            AssignedPlayerNumber
        )
    );

    BroadcastMessage(
        FString::Printf(
            TEXT("플레이어 %d이(가) 게임에 참여했습니다."),
            AssignedPlayerNumber
        )
    );

    TryStartGame();

    if (!bGameInProgress)
    {
        PlayerController->ClientRPCReceiveMessage(
            FString::Printf(
                TEXT("플레이어들을 기다리고 있습니다.. (%d/%d)"),
                GetSortedPlayerStates().Num(),
                MinimumPlayersToStart
            )
        );
    }
}

void ANBGameModeBase::Logout(AController* Exiting)
{
    int32 LeavingPlayerNumber = 0;

    if (const ANBPlayerState* LeavingPlayerState =
        Exiting->GetPlayerState<ANBPlayerState>())
    {
        LeavingPlayerNumber = LeavingPlayerState->GetPlayerNumber();
    }

    const ANBGameStateBase* TurnGameState =
        GetGameState<ANBGameStateBase>();
    const bool bWasCurrentPlayer = IsValid(TurnGameState)
        && TurnGameState->GetCurrentTurnPlayerNumber()
            == LeavingPlayerNumber;

    Super::Logout(Exiting);

    BroadcastMessage(
        FString::Printf(
            TEXT("플레이어 %d이(가) 게임을 떠났습니다."),
            LeavingPlayerNumber
        )
    );

    if (GetSortedPlayerStates().Num() < MinimumPlayersToStart)
    {
        bGameInProgress = false;
        StopTurnTimer();

        if (ANBGameStateBase* TurnStateActor =
            GetGameState<ANBGameStateBase>())
        {
            TurnStateActor->ClearTurnState();
        }

        BroadcastMessage(TEXT("다른 플레이어를 기다리고 있습니다."));
        return;
    }

    if (bGameInProgress && bWasCurrentPlayer)
    {
        AdvanceTurn();
    }
}

void ANBGameModeBase::HandleGuess(
    ANBPlayerController* PlayerController,
    const FString& Guess
)
{
    if (!HasAuthority() || !IsValid(PlayerController))
    {
        return;
    }

    if (!bGameInProgress)
    {
        PlayerController->ClientRPCReceiveMessage(
            TEXT("현재 게임이 더 많은 플레이어를 기다리고 있습니다.")
        );
        return;
    }

    if (!IsPlayersTurn(PlayerController))
    {
        PlayerController->ClientRPCReceiveMessage(
            TEXT("당신 차례가 아닙니다.")
        );
        return;
    }

    ANBPlayerState* PlayerState =
        PlayerController->GetPlayerState<ANBPlayerState>();

    if (!IsValid(PlayerState))
    {
        return;
    }

    FString ValidationError;
    if (!ValidateGuess(Guess, ValidationError))
    {
        PlayerController->ClientRPCReceiveMessage(ValidationError);
        return;
    }

    if (!PlayerState->HasRemainingGuesses())
    {
        PlayerController->ClientRPCReceiveMessage(
            TEXT("남은 기회가 없습니다.")
        );
        return;
    }

    if (!PlayerState->TryIncreaseGuessCount())
    {
        return;
    }

    StopTurnTimer();

    int32 StrikeCount = 0;
    int32 BallCount = 0;
    JudgeGuess(Guess, StrikeCount, BallCount);

    const FString Result = StrikeCount == 0 && BallCount == 0
        ? TEXT("OUT")
        : FString::Printf(TEXT("%dS %dB"), StrikeCount, BallCount);

    const FString StyledResult = FString::Printf(
        TEXT("<Result>%s</>"),
        *Result
    );

    BroadcastRichMessage(
        FString::Printf(
            TEXT("플레이어 %d: %s -> %s [%d/%d]"),
            PlayerState->GetPlayerNumber(),
            *Guess,
            *StyledResult,
            PlayerState->GetCurrentGuessCount(),
            PlayerState->GetMaxGuessCount()
        )
    );

    if (StrikeCount == 3)
    {
        FinishGameWithWinner(PlayerState);
        return;
    }

    if (AreAllPlayersOutOfGuesses())
    {
        FinishGameAsDraw();
        return;
    }

    AdvanceTurn();
}

void ANBGameModeBase::TryStartGame()
{
    if (bGameInProgress
        || GetWorldTimerManager().IsTimerActive(ResetTimerHandle))
    {
        return;
    }

    const TArray<ANBPlayerState*> PlayerStates =
        GetSortedPlayerStates();

    if (PlayerStates.Num() < MinimumPlayersToStart)
    {
        return;
    }

    bGameInProgress = true;
    BroadcastMessage(TEXT("게임이 시작되었습니다."));
    StartTurn(PlayerStates[0]->GetPlayerNumber());
}

void ANBGameModeBase::StartTurn(const int32 PlayerNumber)
{
    ANBGameStateBase* TurnGameState =
        GetGameState<ANBGameStateBase>();

    if (!IsValid(TurnGameState))
    {
        return;
    }

    StopTurnTimer();

    const float TurnEndServerTime =
        TurnGameState->GetServerWorldTimeSeconds()
        + TurnDurationSeconds;

    TurnGameState->SetTurnState(
        PlayerNumber,
        TurnEndServerTime
    );

    GetWorldTimerManager().SetTimer(
        TurnTimerHandle,
        this,
        &ThisClass::HandleTurnTimeout,
        TurnDurationSeconds,
        false
    );

    BroadcastMessage(
        FString::Printf(
            TEXT("플레이어 %d의 차례입니다. %.0f초 남았습니다."),
            PlayerNumber,
            TurnDurationSeconds
        )
    );
}

void ANBGameModeBase::AdvanceTurn()
{
    const TArray<ANBPlayerState*> PlayerStates =
        GetSortedPlayerStates();

    if (PlayerStates.IsEmpty())
    {
        return;
    }

    const ANBGameStateBase* TurnGameState =
        GetGameState<ANBGameStateBase>();
    const int32 CurrentPlayerNumber = IsValid(TurnGameState)
        ? TurnGameState->GetCurrentTurnPlayerNumber()
        : 0;

    int32 CurrentIndex = INDEX_NONE;
    for (int32 Index = 0; Index < PlayerStates.Num(); ++Index)
    {
        if (PlayerStates[Index]->GetPlayerNumber()
            == CurrentPlayerNumber)
        {
            CurrentIndex = Index;
            break;
        }
    }

    for (int32 Offset = 1; Offset <= PlayerStates.Num(); ++Offset)
    {
        const int32 CandidateIndex =
            (CurrentIndex + Offset) % PlayerStates.Num();
        ANBPlayerState* Candidate = PlayerStates[CandidateIndex];

        if (Candidate->HasRemainingGuesses())
        {
            StartTurn(Candidate->GetPlayerNumber());
            return;
        }
    }

    FinishGameAsDraw();
}

void ANBGameModeBase::HandleTurnTimeout()
{
    const ANBGameStateBase* TurnGameState =
        GetGameState<ANBGameStateBase>();

    if (!bGameInProgress || !IsValid(TurnGameState))
    {
        return;
    }

    const int32 TimedOutPlayerNumber =
        TurnGameState->GetCurrentTurnPlayerNumber();

    for (ANBPlayerState* PlayerState : GetSortedPlayerStates())
    {
        if (PlayerState->GetPlayerNumber() == TimedOutPlayerNumber)
        {
            PlayerState->TryIncreaseGuessCount();

            BroadcastMessage(
                FString::Printf(
                    TEXT("플레이어 %d이(가) 시간 초과되었습니다. [%d/%d]을(를) 사용한 것 같습니다."),
                    TimedOutPlayerNumber,
                    PlayerState->GetCurrentGuessCount(),
                    PlayerState->GetMaxGuessCount()
                )
            );
            break;
        }
    }

    if (AreAllPlayersOutOfGuesses())
    {
        FinishGameAsDraw();
        return;
    }

    AdvanceTurn();
}

bool ANBGameModeBase::IsPlayersTurn(
    const ANBPlayerController* PlayerController
) const
{
    if (!IsValid(PlayerController))
    {
        return false;
    }

    const ANBPlayerState* PlayerState =
        PlayerController->GetPlayerState<ANBPlayerState>();
    const ANBGameStateBase* TurnGameState =
        GetGameState<ANBGameStateBase>();

    return IsValid(PlayerState)
        && IsValid(TurnGameState)
        && PlayerState->GetPlayerNumber()
            == TurnGameState->GetCurrentTurnPlayerNumber();
}

void ANBGameModeBase::StopTurnTimer()
{
    GetWorldTimerManager().ClearTimer(TurnTimerHandle);
}

FString ANBGameModeBase::GenerateSecretNumber() const
{
    TArray<int32> SelectedDigits;
    SelectedDigits.Reserve(3);

    while (SelectedDigits.Num() < 3)
    {
        const int32 Candidate = FMath::RandRange(1, 9);
        if (!SelectedDigits.Contains(Candidate))
        {
            SelectedDigits.Add(Candidate);
        }
    }

    FString Result;
    for (const int32 Digit : SelectedDigits)
    {
        Result.AppendInt(Digit);
    }

    return Result;
}

bool ANBGameModeBase::ValidateGuess(
    const FString& Guess,
    FString& OutError
) const
{
    if (Guess.Len() != 3)
    {
        OutError = TEXT("정확히 세 자리 숫자를 입력하세요.");
        return false;
    }

    TSet<TCHAR> UniqueDigits;
    for (const TCHAR Character : Guess)
    {
        if (Character < TEXT('1') || Character > TEXT('9'))
        {
            OutError = TEXT("1부터 9까지의 숫자만 허용됩니다.");
            return false;
        }

        if (UniqueDigits.Contains(Character))
        {
            OutError = TEXT("중복된 숫자는 허용되지 않습니다.");
            return false;
        }

        UniqueDigits.Add(Character);
    }

    return true;
}

void ANBGameModeBase::JudgeGuess(
    const FString& Guess,
    int32& OutStrikeCount,
    int32& OutBallCount
) const
{
    OutStrikeCount = 0;
    OutBallCount = 0;

    for (int32 Index = 0; Index < Guess.Len(); ++Index)
    {
        if (Guess[Index] == SecretNumber[Index])
        {
            ++OutStrikeCount;
        }
        else if (SecretNumber.Contains(FString::Chr(Guess[Index])))
        {
            ++OutBallCount;
        }
    }
}

void ANBGameModeBase::BroadcastMessage(const FString& Message) const
{
    for (TActorIterator<ANBPlayerController> It(GetWorld()); It; ++It)
    {
        ANBPlayerController* PlayerController = *It;
        if (IsValid(PlayerController))
        {
            PlayerController->ClientRPCReceiveMessage(Message);
        }
    }
}

void ANBGameModeBase::BroadcastRichMessage(
    const FString& RichMessage
) const
{
    FString PlainMessage = RichMessage;
    PlainMessage.ReplaceInline(TEXT("<Result>"), TEXT(""));
    PlainMessage.ReplaceInline(TEXT("</>"), TEXT(""));

    for (TActorIterator<ANBPlayerController> It(GetWorld()); It; ++It)
    {
        ANBPlayerController* PlayerController = *It;
        if (IsValid(PlayerController))
        {
            PlayerController->ClientRPCReceiveRichMessage(
                RichMessage,
                PlainMessage
            );
        }
    }
}

TArray<ANBPlayerState*> ANBGameModeBase::GetSortedPlayerStates() const
{
    TArray<ANBPlayerState*> Result;

    const ANBGameStateBase* CurrentGameState =
        GetGameState<ANBGameStateBase>();

    if (!IsValid(CurrentGameState))
    {
        return Result;
    }

    for (const TObjectPtr<APlayerState>& BasePlayerState
        : CurrentGameState->PlayerArray)
    {
        ANBPlayerState* PlayerState =
            Cast<ANBPlayerState>(BasePlayerState.Get());

        if (IsValid(PlayerState))
        {
            Result.Add(PlayerState);
        }
    }

    Result.Sort(
        [](const ANBPlayerState& Left, const ANBPlayerState& Right)
        {
            return Left.GetPlayerNumber() < Right.GetPlayerNumber();
        }
    );

    return Result;
}

bool ANBGameModeBase::AreAllPlayersOutOfGuesses() const
{
    const TArray<ANBPlayerState*> PlayerStates =
        GetSortedPlayerStates();

    if (PlayerStates.IsEmpty())
    {
        return false;
    }

    for (const ANBPlayerState* PlayerState : PlayerStates)
    {
        if (PlayerState->HasRemainingGuesses())
        {
            return false;
        }
    }

    return true;
}

void ANBGameModeBase::FinishGameWithWinner(
    ANBPlayerState* WinnerPlayerState
)
{
    if (!HasAuthority()
        || !bGameInProgress
        || !IsValid(WinnerPlayerState))
    {
        return;
    }

    const int32 WinnerPlayerNumber =
        WinnerPlayerState->GetPlayerNumber();

    for (ANBPlayerState* PlayerState : GetSortedPlayerStates())
    {
        PlayerState->SetMatchResult(
            PlayerState == WinnerPlayerState
                ? ENBMatchResult::Win
                : ENBMatchResult::Lose
        );
    }

    for (TActorIterator<ANBPlayerController> It(GetWorld()); It; ++It)
    {
        ANBPlayerController* PlayerController = *It;
        if (!IsValid(PlayerController))
        {
            continue;
        }

        const ANBPlayerState* PlayerState =
            PlayerController->GetPlayerState<ANBPlayerState>();

        if (!IsValid(PlayerState))
        {
            continue;
        }

        if (PlayerState->GetPlayerNumber() == WinnerPlayerNumber)
        {
            PlayerController->ClientRPCReceiveMessage(
                FString::Printf(
                    TEXT("당신이 승리했습니다! 플레이어 %d이(가) 비밀 번호를 추측했습니다."),
                    WinnerPlayerNumber
                )
            );
        }
        else
        {
            PlayerController->ClientRPCReceiveMessage(
                FString::Printf(
                    TEXT("당신이 패배했습니다. 플레이어 %d이(가) 승리했습니다."),
                    WinnerPlayerNumber
                )
            );
        }
    }

    ScheduleGameReset();
}

void ANBGameModeBase::FinishGameAsDraw()
{
    if (!HasAuthority() || !bGameInProgress)
    {
        return;
    }

    for (ANBPlayerState* PlayerState : GetSortedPlayerStates())
    {
        PlayerState->SetMatchResult(ENBMatchResult::Draw);
    }

    BroadcastMessage(TEXT("비겼습니다! 모든 플레이어가 기회를 모두 사용했습니다."));
    ScheduleGameReset();
}

void ANBGameModeBase::ScheduleGameReset()
{
    bGameInProgress = false;
    StopTurnTimer();

    if (ANBGameStateBase* TurnGameState =
        GetGameState<ANBGameStateBase>())
    {
        TurnGameState->ClearTurnState();
    }

    GetWorldTimerManager().SetTimer(
        ResetTimerHandle,
        this,
        &ThisClass::ResetGame,
        GameResetDelaySeconds,
        false
    );
}

void ANBGameModeBase::ResetGame()
{
    StopTurnTimer();
    GetWorldTimerManager().ClearTimer(ResetTimerHandle);

    if (ANBGameStateBase* TurnGameState =
        GetGameState<ANBGameStateBase>())
    {
        TurnGameState->ClearTurnState();
    }

    for (ANBPlayerState* PlayerState : GetSortedPlayerStates())
    {
        PlayerState->ResetGuessCount();
        PlayerState->ResetMatchResult();
    }

    SecretNumber = GenerateSecretNumber();
    bGameInProgress = false;

    UE_LOG(
        LogTemp,
        Log,
        TEXT("새로 제시된 숫자: %s"),
        *SecretNumber
    );

    BroadcastMessage(TEXT("새로운 게임이 시작됐습니다."));
    TryStartGame();
}
