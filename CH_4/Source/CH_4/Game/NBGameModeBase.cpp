#include "NBGameModeBase.h"
#include "Player/NBPlayerController.h"
#include "Player/NBPlayerState.h"

ANBGameModeBase::ANBGameModeBase()
{
    PlayerControllerClass = ANBPlayerController::StaticClass();
    PlayerStateClass = ANBPlayerState::StaticClass();

    // 채팅 UI 중심 프로젝트이므로 기본 Pawn은 당장 필요하지 않습니다.
    DefaultPawnClass = nullptr;
}

void ANBGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(
        LogTemp,
        Log,
        TEXT("NBGameModeBase started. NetMode: %d"),
        static_cast<int32>(GetNetMode())
    );
}
