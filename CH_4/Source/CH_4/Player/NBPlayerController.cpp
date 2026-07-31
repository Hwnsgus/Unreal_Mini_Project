#include "NBPlayerController.h"
#include "Blueprint/UserWidget.h"

void ANBPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 서버에 존재하는 다른 플레이어의 Controller에서는
    // 내 UI를 생성하면 안 됩니다.
    if (!IsLocalController())
    {
        return;
    }

    if (IsValid(MainWidgetClass))
    {
        MainWidget = CreateWidget<UUserWidget>(this, MainWidgetClass);

        if (IsValid(MainWidget))
        {
            MainWidget->AddToViewport();

            FInputModeGameAndUI InputMode;
            InputMode.SetHideCursorDuringCapture(false);
            SetInputMode(InputMode);

            bShowMouseCursor = true;
        }
    }
}