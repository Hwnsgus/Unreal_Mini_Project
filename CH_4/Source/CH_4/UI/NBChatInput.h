#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "NBChatInput.generated.h"

class UEditableTextBox;

UCLASS()
class CH_4_API UNBChatInput : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UEditableTextBox> EditableTextBox_ChatInput;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    UFUNCTION()
    void HandleChatInputCommitted(
        const FText& Text,
        ETextCommit::Type CommitMethod
    );
};
