#include "DataBase/DatabaseSubsystem.h"

#include "ModuleBindings/Tables/UnrealGuessLogTable.g.h"
#include "ModuleBindings/Tables/UnrealPlayerStatsTable.g.h"

namespace
{
    const FString ServerUri =
        TEXT("https://maincloud.spacetimedb.com");

    const FString DatabaseName =
        TEXT("number-baseball-h0106");
}


void UDatabaseSubsystem::Initialize(
    FSubsystemCollectionBase& Collection
)
{
    Super::Initialize(Collection);

    FOnConnectDelegate ConnectDelegate;
    BIND_DELEGATE_SAFE(
        ConnectDelegate,
        this,
        UDatabaseSubsystem,
        HandleConnect
    );

    FOnConnectErrorDelegate ConnectErrorDelegate;
    BIND_DELEGATE_SAFE(
        ConnectErrorDelegate,
        this,
        UDatabaseSubsystem,
        HandleConnectError
    );

    FOnDisconnectDelegate DisconnectDelegate;
    BIND_DELEGATE_SAFE(
        DisconnectDelegate,
        this,
        UDatabaseSubsystem,
        HandleDisconnect
    );

    UDbConnectionBuilder* Builder = UDbConnection::Builder()
        ->WithUri(ServerUri)
        ->WithDatabaseName(DatabaseName)
        ->OnConnect(ConnectDelegate)
        ->OnConnectError(ConnectErrorDelegate)
        ->OnDisconnect(DisconnectDelegate);

    Connection = Builder->Build();
    if (IsValid(Connection))
    {
        Connection->SetAutoTicking(true);
    }
}

void UDatabaseSubsystem::Deinitialize()
{
    if (IsValid(LogSubscription)
        && !LogSubscription->IsEnded())
    {
        LogSubscription->Unsubscribe();
    }
    LogSubscription = nullptr;

    if (IsValid(Connection))
    {
        if (IsValid(Connection->Db)
            && IsValid(Connection->Db->UnrealGuessLog))
        {
            Connection->Db->UnrealGuessLog->OnInsert.RemoveDynamic(
                this,
                &UDatabaseSubsystem::HandleGuessInserted
            );
        }

        Connection->SetAutoTicking(false);
        Connection->Disconnect();
        Connection = nullptr;
    }

    Super::Deinitialize();
}

bool UDatabaseSubsystem::IsConnected() const
{
    return IsValid(Connection) && Connection->IsActive();
}

bool UDatabaseSubsystem::RecordGuess(
    const FString& SessionId,
    const int32 Round,
    const int32 PlayerNumber,
    const FString& Guess,
    const int32 Strikes,
    const int32 Balls,
    const int32 AttemptNumber
)
{
    if (SessionId.IsEmpty()
        || Round < 1
        || (PlayerNumber != 1 && PlayerNumber != 2)
        || AttemptNumber < 1
        || AttemptNumber > 3
        || Strikes < 0
        || Balls < 0
        || Strikes + Balls > 3)
    {
        UE_LOG(LogTemp, Warning, TEXT("Rejected an invalid guess log."));
        return false;
    }

    if (!IsConnected() || !IsValid(Connection->Reducers))
    {
        PendingGuessRecords.Add(
            {
                Round,
                SessionId,
                PlayerNumber,
                Guess,
                Strikes,
                Balls,
                AttemptNumber
            }
        );
        return true;
    }

    Connection->Reducers->RecordUnrealGuess(
        SessionId,
        static_cast<uint32>(Round),
        static_cast<uint32>(PlayerNumber),
        Guess,
        static_cast<uint32>(Strikes),
        static_cast<uint32>(Balls),
        static_cast<uint32>(AttemptNumber)
    );
    return true;
}

bool UDatabaseSubsystem::RecordRoundResult(
    const FString& SessionId,
    const int32 Round,
    const FString& Answer,
    const int32 WinnerPlayerNumber,
    const bool bIsDraw
)
{
    if (SessionId.IsEmpty()
        || Round < 1
        || (!bIsDraw
            && WinnerPlayerNumber != 1
            && WinnerPlayerNumber != 2))
    {
        UE_LOG(LogTemp, Warning, TEXT("Rejected an invalid round log."));
        return false;
    }

    if (!IsConnected() || !IsValid(Connection->Reducers))
    {
        PendingRoundRecords.Add(
            {Round, SessionId, Answer, WinnerPlayerNumber, bIsDraw}
        );
        return true;
    }

    Connection->Reducers->RecordUnrealRoundResult(
        SessionId,
        static_cast<uint32>(Round),
        Answer,
        bIsDraw ? TEXT("draw") : TEXT("won"),
        static_cast<uint32>(bIsDraw ? 0 : WinnerPlayerNumber)
    );
    return true;
}

TArray<FNBGuessLogEntry>
UDatabaseSubsystem::GetAllGuessLogs() const
{
    TArray<FNBGuessLogEntry> Result;

    if (!IsConnected()
        || !IsValid(Connection->Db)
        || !IsValid(Connection->Db->UnrealGuessLog))
    {
        return Result;
    }

    TArray<FUnrealGuessLogType> Rows =
        Connection->Db->UnrealGuessLog->Iter();
    Rows.Sort(
        [](const FUnrealGuessLogType& Left,
           const FUnrealGuessLogType& Right)
        {
            return Left.Id < Right.Id;
        }
    );

    Result.Reserve(Rows.Num());
    for (const FUnrealGuessLogType& Row : Rows)
    {
        Result.Add(MakeLogEntry(Row));
    }

    return Result;
}

TArray<FNBGuessLogEntry>
UDatabaseSubsystem::GetGuessLogsForRound(
    const int32 Round
) const
{
    TArray<FNBGuessLogEntry> Result;

    if (Round < 1
        || !IsConnected()
        || !IsValid(Connection->Db)
        || !IsValid(Connection->Db->UnrealGuessLog)
        || !IsValid(Connection->Db->UnrealGuessLog->Round))
    {
        return Result;
    }

    TArray<FUnrealGuessLogType> Rows =
        Connection->Db->UnrealGuessLog->Round->Filter(
            static_cast<uint32>(Round)
        );
    Rows.Sort(
        [](const FUnrealGuessLogType& Left,
           const FUnrealGuessLogType& Right)
        {
            return Left.Id < Right.Id;
        }
    );

    Result.Reserve(Rows.Num());
    for (const FUnrealGuessLogType& Row : Rows)
    {
        Result.Add(MakeLogEntry(Row));
    }

    return Result;
}

int32 UDatabaseSubsystem::GetPlayerWins(
    const int32 PlayerNumber
) const
{
    if ((PlayerNumber != 1 && PlayerNumber != 2)
        || !IsConnected()
        || !IsValid(Connection->Db)
        || !IsValid(Connection->Db->UnrealPlayerStats))
    {
        return 0;
    }

    for (const FUnrealPlayerStatsType& Stats
        : Connection->Db->UnrealPlayerStats->Iter())
    {
        if (Stats.PlayerNumber == static_cast<uint32>(PlayerNumber))
        {
            return static_cast<int32>(Stats.Wins);
        }
    }

    return 0;
}

FNBGuessLogEntry UDatabaseSubsystem::MakeLogEntry(
    const FUnrealGuessLogType& Row
)
{
    FNBGuessLogEntry Entry;
    Entry.RecordId = static_cast<int64>(Row.Id);
    Entry.SessionId = Row.SessionId;
    Entry.Round = static_cast<int32>(Row.Round);
    Entry.PlayerNumber = static_cast<int32>(Row.PlayerNumber);
    Entry.PlayerName = FString::Printf(
        TEXT("Player %d"),
        Entry.PlayerNumber
    );
    Entry.Guess = Row.Guess;
    Entry.Strikes = static_cast<int32>(Row.Strikes);
    Entry.Balls = static_cast<int32>(Row.Balls);
    Entry.bIsOut = Row.IsOut;
    Entry.AttemptNumber = static_cast<int32>(Row.AttemptNumber);
    Entry.CreatedAt = Row.CreatedAt.ToString();
    return Entry;
}

void UDatabaseSubsystem::FlushPendingRecords()
{
    if (!IsConnected() || !IsValid(Connection->Reducers))
    {
        return;
    }

    const TArray<FPendingGuessRecord> Guesses =
        MoveTemp(PendingGuessRecords);
    PendingGuessRecords.Reset();
    for (const FPendingGuessRecord& Record : Guesses)
    {
        RecordGuess(
            Record.SessionId,
            Record.Round,
            Record.PlayerNumber,
            Record.Guess,
            Record.Strikes,
            Record.Balls,
            Record.AttemptNumber
        );
    }

    const TArray<FPendingRoundRecord> Rounds =
        MoveTemp(PendingRoundRecords);
    PendingRoundRecords.Reset();
    for (const FPendingRoundRecord& Record : Rounds)
    {
        RecordRoundResult(
            Record.SessionId,
            Record.Round,
            Record.Answer,
            Record.WinnerPlayerNumber,
            Record.bIsDraw
        );
    }
}

void UDatabaseSubsystem::HandleConnect(
    UDbConnection* InConnection,
    FSpacetimeDBIdentity Identity,
    const FString& Token
)
{
    UE_LOG(
        LogTemp,
        Log,
        TEXT("Connected to SpacetimeDB database '%s'."),
        *DatabaseName
    );

    if (!IsValid(InConnection)
        || !IsValid(InConnection->Db)
        || !IsValid(InConnection->Db->UnrealGuessLog))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("The generated Unreal log table binding is unavailable.")
        );
        return;
    }

    InConnection->Db->UnrealGuessLog->OnInsert.AddUniqueDynamic(
        this,
        &UDatabaseSubsystem::HandleGuessInserted
    );

    FOnSubscriptionApplied AppliedDelegate;
    BIND_DELEGATE_SAFE(
        AppliedDelegate,
        this,
        UDatabaseSubsystem,
        HandleSubscriptionApplied
    );

    FOnSubscriptionError ErrorDelegate;
    BIND_DELEGATE_SAFE(
        ErrorDelegate,
        this,
        UDatabaseSubsystem,
        HandleSubscriptionError
    );

    TArray<FString> Queries;
    Queries.Add(TEXT("SELECT * FROM unreal_guess_log"));
    Queries.Add(TEXT("SELECT * FROM unreal_round_result"));
    Queries.Add(TEXT("SELECT * FROM unreal_player_stats"));

    LogSubscription = InConnection->SubscriptionBuilder()
        ->OnApplied(AppliedDelegate)
        ->OnError(ErrorDelegate)
        ->Subscribe(Queries);

    FlushPendingRecords();
}

void UDatabaseSubsystem::HandleConnectError(
    const FString& Error
)
{
    UE_LOG(
        LogTemp,
        Error,
        TEXT("SpacetimeDB connection failed: %s"),
        *Error
    );
}

void UDatabaseSubsystem::HandleDisconnect(
    UDbConnection* InConnection,
    const FString& Error
)
{
    if (Error.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("Disconnected from SpacetimeDB."));
        return;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("SpacetimeDB disconnected: %s"),
        *Error
    );
}

void UDatabaseSubsystem::HandleSubscriptionApplied(
    FSubscriptionEventContext Context
)
{
    const int32 LogCount = IsValid(Context.Db)
        && IsValid(Context.Db->UnrealGuessLog)
        ? Context.Db->UnrealGuessLog->Count()
        : 0;

    UE_LOG(
        LogTemp,
        Log,
        TEXT("Guess log subscription ready. Cached rows: %d"),
        LogCount
    );

    OnGuessLogsReady.Broadcast();
}

void UDatabaseSubsystem::HandleSubscriptionError(
    FErrorContext Context
)
{
    UE_LOG(
        LogTemp,
        Error,
        TEXT("Guess log subscription failed: %s"),
        *Context.Error
    );
}

void UDatabaseSubsystem::HandleGuessInserted(
    const FEventContext& Context,
    const FUnrealGuessLogType& NewRow
)
{
    const FNBGuessLogEntry Entry = MakeLogEntry(NewRow);

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[Round %d] %s: %s -> %dS %dB"),
        Entry.Round,
        *Entry.PlayerName,
        *Entry.Guess,
        Entry.Strikes,
        Entry.Balls
    );

    OnGuessLogAdded.Broadcast(Entry);
}
