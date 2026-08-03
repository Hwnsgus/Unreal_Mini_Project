#include "NBPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Game/NBGameModeBase.h"

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

    if (IsValid(NotificationTextWidgetClass))
    {
        NotificationTextWidgetInstance = CreateWidget<UUserWidget>(
            this,
            NotificationTextWidgetClass
        );

        if (IsValid(NotificationTextWidgetInstance))
        {
            NotificationTextWidgetInstance->AddToViewport(10);
        }
    }
}

void ANBPlayerController::SubmitGuess(const FString& Guess)
{
    if (!IsLocalController())
    {
        return;
    }

    ServerRPCSubmitGuess(Guess);
}

void ANBPlayerController::ServerRPCSubmitGuess_Implementation(
    const FString& Guess
)
{
    ANBGameModeBase* GameMode =
        GetWorld()->GetAuthGameMode<ANBGameModeBase>();

    if (IsValid(GameMode))
    {
        GameMode->HandleGuess(this, Guess);
    }
}

void ANBPlayerController::ClientRPCReceiveMessage_Implementation(
    const FString& Message
)
{
    NotificationText = FText::FromString(Message);
    PrintMessage(Message);
}

FText ANBPlayerController::GetNotificationText() const
{
    return NotificationText;
}

void ANBPlayerController::PrintMessage(const FString& Message) const
{
    UE_LOG(LogTemp, Log, TEXT("%s"), *Message);

    if (IsValid(GEngine))
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            6.0f,
            FColor::Cyan,
            Message
        );
    }
}
