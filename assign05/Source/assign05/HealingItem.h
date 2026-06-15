#pragma once

#include "CoreMinimal.h"
#include "PickupItemBase.h"
#include "HealingItem.generated.h"

UCLASS()
class ASSIGN05_API AHealingItem : public APickupItemBase
{
	GENERATED_BODY()

public:
	AHealingItem();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Healing")
	float HealAmount = 20.0f;

	virtual void ApplyPickupEffect_Implementation(APawn* PickingPawn) override;
};
