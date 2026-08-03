#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NBGameStateBase.generated.h"

USTRUCT(BlueprintType)
struct FNBTurnState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Turn")
    int32 CurrentTurnPlayerNumber = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Turn")
    float TurnEndServerTime = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNBOnTurnStateChanged);

UCLASS()
class CH_4_API ANBGameStateBase : public AGameStateBase
{
    GENERATED_BODY()

public:
    ANBGameStateBase();

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    void SetTurnState(int32 PlayerNumber, float TurnEndTime);
    void ClearTurnState();

    UFUNCTION(BlueprintPure, Category = "Turn")
    int32 GetCurrentTurnPlayerNumber() const;

    UFUNCTION(BlueprintPure, Category = "Turn")
    float GetRemainingTurnTime() const;

    UFUNCTION(BlueprintPure, Category = "Turn")
    bool IsTurnActive() const;

    UPROPERTY(BlueprintAssignable, Category = "Turn")
    FNBOnTurnStateChanged OnTurnStateChanged;

private:
    UFUNCTION()
    void OnRep_TurnState();

    UPROPERTY(ReplicatedUsing = OnRep_TurnState)
    FNBTurnState TurnState;
};
