#include "MineItem.h"

#include "Assign05Character.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AMineItem::AMineItem()
{
	ScoreValue = 0;
	bCountsTowardWaveGoal = false;
	bDestroyOnPickup = true;
}

void AMineItem::ApplyPickupEffect_Implementation(APawn* PickingPawn)
{
	if (AAssign05Character* Character = Cast<AAssign05Character>(PickingPawn))
	{
		UGameplayStatics::ApplyDamage(Character, DamageAmount, nullptr, this, nullptr);
		Character->ApplyCameraReverseDebuff(CameraReverseDuration);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Mine debuff activated!"));
	}
}
