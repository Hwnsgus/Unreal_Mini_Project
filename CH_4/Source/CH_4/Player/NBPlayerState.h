#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NBPlayerState.generated.h"

UCLASS()
class CH_4_API ANBPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    ANBPlayerState();

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    // 아래 변경 함수들은 서버에서만 상태를 수정합니다.
    void SetPlayerNumber(int32 InPlayerNumber);
    bool TryIncreaseGuessCount();
    void ResetGuessCount();

    UFUNCTION(BlueprintPure, Category = "Number Baseball")
    int32 GetPlayerNumber() const;

    UFUNCTION(BlueprintPure, Category = "Number Baseball")
    int32 GetCurrentGuessCount() const;

    UFUNCTION(BlueprintPure, Category = "Number Baseball")
    int32 GetMaxGuessCount() const;

    UFUNCTION(BlueprintPure, Category = "Number Baseball")
    int32 GetRemainingGuessCount() const;

    UFUNCTION(BlueprintPure, Category = "Number Baseball")
    bool HasRemainingGuesses() const;

private:
    UPROPERTY(
        Replicated,
        VisibleInstanceOnly,
        BlueprintReadOnly,
        Category = "Number Baseball",
        meta = (AllowPrivateAccess = "true")
    )
    int32 PlayerNumber;

    UPROPERTY(
        Replicated,
        VisibleInstanceOnly,
        BlueprintReadOnly,
        Category = "Number Baseball",
        meta = (AllowPrivateAccess = "true")
    )
    int32 CurrentGuessCount;

    UPROPERTY(
        Replicated,
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Number Baseball",
        meta = (AllowPrivateAccess = "true", ClampMin = "1")
    )
    int32 MaxGuessCount;
};
