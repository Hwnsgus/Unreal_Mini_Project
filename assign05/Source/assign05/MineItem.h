#pragma once

#include "PickupItemBase.h"
#include "MineItem.generated.h"

UCLASS()
class ASSIGN05_API AMineItem : public APickupItemBase
{
	GENERATED_BODY()

public:
	AMineItem();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mine")
	float CameraReverseDuration = 5.0f;

	virtual void ApplyPickupEffect_Implementation(APawn* PickingPawn) override;
};
