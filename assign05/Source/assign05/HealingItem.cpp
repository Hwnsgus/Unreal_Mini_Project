#include "HealingItem.h"

#include "Assign05Character.h"

AHealingItem::AHealingItem()
{
	ScoreValue = 10;
}

void AHealingItem::ApplyPickupEffect_Implementation(APawn* PickingPawn)
{
	if (AAssign05Character* Character = Cast<AAssign05Character>(PickingPawn))
	{
		Character->AddHealth(HealAmount);
		
	}
}
