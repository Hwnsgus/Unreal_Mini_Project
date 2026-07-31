#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NBPlayerController.generated.h"

class UUserWidget;

UCLASS()
class CH_4_API ANBPlayerController : public APlayerController
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> MainWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> MainWidget;
};