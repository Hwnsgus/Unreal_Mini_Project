#pragma once

#include "CoreMinimal.h"
#include "ModuleBindings/SpacetimeDBClient.g.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DatabaseSubsystem.generated.h"

USTRUCT(BlueprintType)
struct CH_4_API FNBGuessLogEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "SpacetimeDB|Log")
    int64 RecordId = 0;

    UPROPERTY(BlueprintReadOnly, Category = "SpacetimeDB|Log")
    FString SessionId;

    UPROPERTY(BlueprintReadOnly, Category = "SpacetimeDB|Log")
    int32 Round = 0;

    UPROPERTY(BlueprintReadOnly, Category = "SpacetimeDB|Log")
    int32 PlayerNumber = 0;

    UPROPERTY(BlueprintReadOnly, Category = "SpacetimeDB|Log")
    FString PlayerName;

    UPROPERTY(BlueprintReadOnly, Category = "SpacetimeDB|Log")
    FString Guess;

    UPROPERTY(BlueprintReadOnly, Category = "SpacetimeDB|Log")
    int32 Strikes = 0;

    UPROPERTY(BlueprintReadOnly, Category = "SpacetimeDB|Log")
    int32 Balls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "SpacetimeDB|Log")
    bool bIsOut = false;

    UPROPERTY(BlueprintReadOnly, Category = "SpacetimeDB|Log")
    int32 AttemptNumber = 0;

    UPROPERTY(BlueprintReadOnly, Category = "SpacetimeDB|Log")
    FString CreatedAt;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FNBOnGuessLogAdded,
    const FNBGuessLogEntry&,
    LogEntry
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNBOnGuessLogsReady);

UCLASS(BlueprintType)
class CH_4_API UDatabaseSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(
        FSubsystemCollectionBase& Collection
    ) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintPure, Category = "SpacetimeDB")
    bool IsConnected() const;

    UFUNCTION(BlueprintCallable, Category = "SpacetimeDB|Log")
    bool RecordGuess(
        const FString& SessionId,
        int32 Round,
        int32 PlayerNumber,
        const FString& Guess,
        int32 Strikes,
        int32 Balls,
        int32 AttemptNumber
    );

    UFUNCTION(BlueprintCallable, Category = "SpacetimeDB|Log")
    bool RecordRoundResult(
        const FString& SessionId,
        int32 Round,
        const FString& Answer,
        int32 WinnerPlayerNumber,
        bool bIsDraw
    );

    UFUNCTION(BlueprintPure, Category = "SpacetimeDB|Log")
    TArray<FNBGuessLogEntry> GetAllGuessLogs() const;

    UFUNCTION(BlueprintPure, Category = "SpacetimeDB|Log")
    TArray<FNBGuessLogEntry> GetGuessLogsForRound(
        int32 Round
    ) const;

    UFUNCTION(BlueprintPure, Category = "SpacetimeDB|Log")
    int32 GetPlayerWins(int32 PlayerNumber) const;

    UPROPERTY(BlueprintAssignable, Category = "SpacetimeDB|Log")
    FNBOnGuessLogAdded OnGuessLogAdded;

    UPROPERTY(BlueprintAssignable, Category = "SpacetimeDB|Log")
    FNBOnGuessLogsReady OnGuessLogsReady;

private:
    static FNBGuessLogEntry MakeLogEntry(
        const FUnrealGuessLogType& Row
    );

    void FlushPendingRecords();

    UFUNCTION()
    void HandleConnect(
        UDbConnection* InConnection,
        FSpacetimeDBIdentity Identity,
        const FString& Token
    );

    UFUNCTION()
    void HandleConnectError(const FString& Error);

    UFUNCTION()
    void HandleDisconnect(
        UDbConnection* InConnection,
        const FString& Error
    );

    UFUNCTION()
    void HandleSubscriptionApplied(
        FSubscriptionEventContext Context
    );

    UFUNCTION()
    void HandleSubscriptionError(FErrorContext Context);

    UFUNCTION()
    void HandleGuessInserted(
        const FEventContext& Context,
        const FUnrealGuessLogType& NewRow
    );

    struct FPendingGuessRecord
    {
        int32 Round = 0;
        FString SessionId;
        int32 PlayerNumber = 0;
        FString Guess;
        int32 Strikes = 0;
        int32 Balls = 0;
        int32 AttemptNumber = 0;
    };

    struct FPendingRoundRecord
    {
        int32 Round = 0;
        FString SessionId;
        FString Answer;
        int32 WinnerPlayerNumber = 0;
        bool bIsDraw = false;
    };

    TArray<FPendingGuessRecord> PendingGuessRecords;
    TArray<FPendingRoundRecord> PendingRoundRecords;

    UPROPERTY()
    TObjectPtr<UDbConnection> Connection;

    UPROPERTY()
    TObjectPtr<USubscriptionHandle> LogSubscription;
};
