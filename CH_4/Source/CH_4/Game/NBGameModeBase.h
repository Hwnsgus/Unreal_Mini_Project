#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"
#include "NBGameModeBase.generated.h"

class ANBPlayerController;
class ANBPlayerState;

UCLASS()
class CH_4_API ANBGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    ANBGameModeBase();

    virtual void OnPostLogin(AController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    void HandleGuess(
        ANBPlayerController* PlayerController,
        const FString& Guess
    );

protected:
    virtual void BeginPlay() override;

private:
    FString GenerateSecretNumber() const;
    bool ValidateGuess(const FString& Guess, FString& OutError) const;
    void JudgeGuess(
        const FString& Guess,
        int32& OutStrikeCount,
        int32& OutBallCount
    ) const;

    void TryStartGame();
    void StartTurn(int32 PlayerNumber);
    void AdvanceTurn();
    void HandleTurnTimeout();
    bool IsPlayersTurn(const ANBPlayerController* PlayerController) const;
    void StopTurnTimer();

    void BroadcastMessage(
    const FString& Message,
    const FColor& MessageColor = FColor::Cyan
    ) const;

    void BroadcastRichMessage(
        const FString& RichMessage,
        const FColor& MessageColor = FColor::Cyan
    ) const;
    TArray<ANBPlayerState*> GetSortedPlayerStates() const;
    bool AreAllPlayersOutOfGuesses() const;
    void FinishGameWithWinner(ANBPlayerState* WinnerPlayerState);
    void FinishGameAsDraw();
    void ScheduleGameReset();
    void ResetGame();

    UPROPERTY(EditDefaultsOnly, Category = "Turn", meta = (ClampMin = "1.0"))
    float TurnDurationSeconds;

    UPROPERTY(EditDefaultsOnly, Category = "Turn", meta = (ClampMin = "1"))
    int32 MinimumPlayersToStart;

    UPROPERTY(EditDefaultsOnly, Category = "Game", meta = (ClampMin = "0.1"))
    float GameResetDelaySeconds;

    FString SecretNumber;
    int32 NextPlayerNumber;
    bool bGameInProgress;
    FTimerHandle TurnTimerHandle;
    FTimerHandle ResetTimerHandle;
};
