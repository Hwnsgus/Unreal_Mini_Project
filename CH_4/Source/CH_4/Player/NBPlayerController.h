#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NBPlayerController.generated.h"

class UUserWidget;

UCLASS()
class CH_4_API ANBPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    void SubmitGuess(const FString& Guess);

    UFUNCTION(Client, Reliable)
    void ClientRPCReceiveMessage(const FString& Message);

    UFUNCTION(BlueprintPure, Category = "UI")
    FText GetNotificationText() const;

    UPROPERTY(BlueprintReadOnly, Category = "UI")
    FText NotificationText;

protected:
    virtual void BeginPlay() override;

    UFUNCTION(Server, Reliable)
    void ServerRPCSubmitGuess(const FString& Guess);

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> MainWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> MainWidget;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> NotificationTextWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> NotificationTextWidgetInstance;

private:
    void PrintMessage(const FString& Message) const;
};
