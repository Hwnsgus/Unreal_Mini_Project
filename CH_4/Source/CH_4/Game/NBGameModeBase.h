#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NBGameModeBase.generated.h"

UCLASS()
class CH_4_API ANBGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    ANBGameModeBase();

protected:
    virtual void BeginPlay() override;
};