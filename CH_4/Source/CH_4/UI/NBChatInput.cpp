#include "UI/NBChatInput.h"

#include "Player/NBPlayerController.h"

void UNBChatInput::NativeConstruct()
{
    Super::NativeConstruct();

    if (IsValid(EditableTextBox_ChatInput))
    {
        EditableTextBox_ChatInput->OnTextCommitted.AddUniqueDynamic(
            this,
            &ThisClass::HandleChatInputCommitted
        );
    }
}

void UNBChatInput::NativeDestruct()
{
    if (IsValid(EditableTextBox_ChatInput))
    {
        EditableTextBox_ChatInput->OnTextCommitted.RemoveDynamic(
            this,
            &ThisClass::HandleChatInputCommitted
        );
    }

    Super::NativeDestruct();
}

void UNBChatInput::HandleChatInputCommitted(
    const FText& Text,
    const ETextCommit::Type CommitMethod
)
{
    if (CommitMethod != ETextCommit::OnEnter)
    {
        return;
    }

    const FString Guess = Text.ToString().TrimStartAndEnd();
    if (Guess.IsEmpty())
    {
        return;
    }

    ANBPlayerController* PlayerController =
        Cast<ANBPlayerController>(GetOwningPlayer());

    if (IsValid(PlayerController))
    {
        PlayerController->SubmitGuess(Guess);
    }

    EditableTextBox_ChatInput->SetText(FText::GetEmpty());
    EditableTextBox_ChatInput->SetKeyboardFocus();
}
