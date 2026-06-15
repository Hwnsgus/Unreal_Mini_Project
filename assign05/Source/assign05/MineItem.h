#pragma once

#include "CoreMinimal.h"
#include "PickupItemBase.h"
#include "MineItem.generated.h"

class USphereComponent;

UCLASS()
class ASSIGN05_API AMineItem : public APickupItemBase
{
	GENERATED_BODY()

public:
	AMineItem();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Component")
	TObjectPtr<USphereComponent> ExplosionCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mine")
	float ExplosionDelay = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mine")
	float ExplosionRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mine")
	float ExplosionDamage = 30.0f;

	FTimerHandle ExplosionTimerHandle;
	bool bExplosionStarted = false;

	virtual void ApplyPickupEffect_Implementation(APawn* PickingPawn) override;

	void Explode();
};
