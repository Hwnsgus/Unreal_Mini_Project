#include "Game/NBGameModeBase.h"

#include "EngineUtils.h"
#include "Game/NBGameStateBase.h"
#include "Player/NBPlayerController.h"
#include "Player/NBPlayerState.h"

ANBGameModeBase::ANBGameModeBase()
    : TurnDurationSeconds(15.0f)
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
        TEXT("Number baseball started. Secret number: %s, NetMode: %d"),
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
            TEXT("You are Player %d. Enter three unique digits from 1 to 9."),
            AssignedPlayerNumber
        )
    );

    BroadcastMessage(
        FString::Printf(
            TEXT("Player %d joined the game."),
            AssignedPlayerNumber
        )
    );

    TryStartGame();

    if (!bGameInProgress)
    {
        PlayerController->ClientRPCReceiveMessage(
            FString::Printf(
                TEXT("Waiting for players... (%d/%d)"),
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
            TEXT("Player %d left the game."),
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

        BroadcastMessage(TEXT("Waiting for another player."));
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
            TEXT("The game is waiting for more players.")
        );
        return;
    }

    if (!IsPlayersTurn(PlayerController))
    {
        PlayerController->ClientRPCReceiveMessage(
            TEXT("It is not your turn.")
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
            TEXT("You have no guesses remaining.")
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

    BroadcastMessage(
        FString::Printf(
            TEXT("Player %d: %s -> %s [%d/%d]"),
            PlayerState->GetPlayerNumber(),
            *Guess,
            *Result,
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
    BroadcastMessage(TEXT("The game has started."));
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
            TEXT("Player %d's turn. %.0f seconds remaining."),
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
                    TEXT("Player %d timed out. Guess used [%d/%d]."),
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
        OutError = TEXT("Enter exactly three digits.");
        return false;
    }

    TSet<TCHAR> UniqueDigits;
    for (const TCHAR Character : Guess)
    {
        if (Character < TEXT('1') || Character > TEXT('9'))
        {
            OutError = TEXT("Only digits from 1 to 9 are allowed.");
            return false;
        }

        if (UniqueDigits.Contains(Character))
        {
            OutError = TEXT("Duplicate digits are not allowed.");
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
    if (!HasAuthority() || !IsValid(WinnerPlayerState))
    {
        return;
    }

    const int32 WinnerPlayerNumber =
        WinnerPlayerState->GetPlayerNumber();

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
                    TEXT("YOU WIN! Player %d guessed the secret number."),
                    WinnerPlayerNumber
                )
            );
        }
        else
        {
            PlayerController->ClientRPCReceiveMessage(
                FString::Printf(
                    TEXT("YOU LOSE. Player %d wins."),
                    WinnerPlayerNumber
                )
            );
        }
    }

    ScheduleGameReset();
}

void ANBGameModeBase::FinishGameAsDraw()
{
    if (!HasAuthority())
    {
        return;
    }

    BroadcastMessage(TEXT("DRAW! All players used every guess."));
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
    }

    SecretNumber = GenerateSecretNumber();
    bGameInProgress = false;

    UE_LOG(
        LogTemp,
        Log,
        TEXT("New secret number: %s"),
        *SecretNumber
    );

    BroadcastMessage(TEXT("A new game has started."));
    TryStartGame();
}
