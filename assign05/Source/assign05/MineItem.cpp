#include "MineItem.h"

#include "Assign05Character.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AMineItem::AMineItem()
{
	ScoreValue = 0;
	bCountsTowardWaveGoal = false;
	bDestroyOnPickup = false;

	ExplosionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionCollision"));
	ExplosionCollision->InitSphereRadius(ExplosionRadius);
	ExplosionCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	ExplosionCollision->SetupAttachment(RootComponent);
}

void AMineItem::ApplyPickupEffect_Implementation(APawn* PickingPawn)
{
	if (bExplosionStarted)
	{
		return;
	}

	bExplosionStarted = true;
	GetWorld()->GetTimerManager().SetTimer(ExplosionTimerHandle, this, &AMineItem::Explode, ExplosionDelay);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Mine activated!"));
	}
}

void AMineItem::Explode()
{
	TArray<AActor*> OverlappingActors;
	ExplosionCollision->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && (Actor->ActorHasTag("Player") || Actor->IsA<AAssign05Character>()))
		{
			//언리얼에서 제공하는 대미지 시스템
			UGameplayStatics::ApplyDamage(Actor, ExplosionDamage, nullptr, this, UDamageType::StaticClass());

			
		}
	}

	Destroy();
}
