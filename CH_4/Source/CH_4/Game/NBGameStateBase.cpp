#include "Game/NBGameStateBase.h"

#include "Net/UnrealNetwork.h"

ANBGameStateBase::ANBGameStateBase()
{
    bReplicates = true;
}

void ANBGameStateBase::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, TurnState);
}

void ANBGameStateBase::SetTurnState(
    const int32 PlayerNumber,
    const float TurnEndTime
)
{
    if (!HasAuthority())
    {
        return;
    }

    TurnState.CurrentTurnPlayerNumber = PlayerNumber;
    TurnState.TurnEndServerTime = TurnEndTime;
    ForceNetUpdate();
    OnTurnStateChanged.Broadcast();
}

void ANBGameStateBase::ClearTurnState()
{
    SetTurnState(0, 0.0f);
}

int32 ANBGameStateBase::GetCurrentTurnPlayerNumber() const
{
    return TurnState.CurrentTurnPlayerNumber;
}

float ANBGameStateBase::GetRemainingTurnTime() const
{
    if (!IsTurnActive())
    {
        return 0.0f;
    }

    return FMath::Max(
        0.0f,
        TurnState.TurnEndServerTime - GetServerWorldTimeSeconds()
    );
}

bool ANBGameStateBase::IsTurnActive() const
{
    return TurnState.CurrentTurnPlayerNumber > 0
        && TurnState.TurnEndServerTime > 0.0f;
}

void ANBGameStateBase::OnRep_TurnState()
{
    OnTurnStateChanged.Broadcast();
}
