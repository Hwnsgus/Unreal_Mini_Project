#include "Player/NBPlayerState.h"

#include "Net/UnrealNetwork.h"

ANBPlayerState::ANBPlayerState()
    : PlayerNumber(0)
    , CurrentGuessCount(0)
    , MaxGuessCount(3)
{
    bReplicates = true;
}

void ANBPlayerState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, PlayerNumber);
    DOREPLIFETIME(ThisClass, CurrentGuessCount);
    DOREPLIFETIME(ThisClass, MaxGuessCount);
}

void ANBPlayerState::SetPlayerNumber(const int32 InPlayerNumber)
{
    if (!HasAuthority())
    {
        return;
    }

    PlayerNumber = FMath::Max(0, InPlayerNumber);
    ForceNetUpdate();
}

bool ANBPlayerState::TryIncreaseGuessCount()
{
    if (!HasAuthority() || !HasRemainingGuesses())
    {
        return false;
    }

    ++CurrentGuessCount;
    ForceNetUpdate();
    return true;
}

void ANBPlayerState::ResetGuessCount()
{
    if (!HasAuthority())
    {
        return;
    }

    CurrentGuessCount = 0;
    ForceNetUpdate();
}

int32 ANBPlayerState::GetPlayerNumber() const
{
    return PlayerNumber;
}

int32 ANBPlayerState::GetCurrentGuessCount() const
{
    return CurrentGuessCount;
}

int32 ANBPlayerState::GetMaxGuessCount() const
{
    return MaxGuessCount;
}

int32 ANBPlayerState::GetRemainingGuessCount() const
{
    return FMath::Max(0, MaxGuessCount - CurrentGuessCount);
}

bool ANBPlayerState::HasRemainingGuesses() const
{
    return CurrentGuessCount < MaxGuessCount;
}
