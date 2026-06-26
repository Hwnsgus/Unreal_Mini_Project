// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpawnVolume.h"

#include "Components/BoxComponent.h"
#include "HealingItem.h"
#include "Kismet/KismetMathLibrary.h"
#include "MineItem.h"
#include "PickupItemBase.h"

ASpawnVolume::ASpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBounds"));
	SetRootComponent(SpawnBounds);
	SpawnBounds->SetBoxExtent(FVector(500.0f, 500.0f, 120.0f));
	SpawnBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnBounds->ShapeColor = FColor::Green;

	RequiredPickupClass = APickupItemBase::StaticClass();
	SpawnableClasses.Add(APickupItemBase::StaticClass());
	SpawnableClasses.Add(AHealingItem::StaticClass());
	SpawnableClasses.Add(AMineItem::StaticClass());
}

void ASpawnVolume::SpawnWaveItems(int32 SpawnCount)
{
	if (SpawnCount <= 0 || SpawnableClasses.Num() == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (int32 Index = 0; Index < SpawnCount; ++Index)
	{
		const int32 ClassIndex = FMath::RandRange(0, SpawnableClasses.Num() - 1);
		TSubclassOf<AActor> SpawnClass = SpawnableClasses[ClassIndex];
		if (SpawnClass == nullptr)
		{
			continue;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		World->SpawnActor<AActor>(SpawnClass, GetRandomPointInVolume(), FRotator::ZeroRotator, SpawnParams);
	}
}

void ASpawnVolume::SpawnRequiredPickupItems(int32 SpawnCount)
{
	SpawnItemsOfClass(RequiredPickupClass, SpawnCount);
}

void ASpawnVolume::SpawnItemsOfClass(TSubclassOf<AActor> SpawnClass, int32 SpawnCount)
{
	if (SpawnCount <= 0 || SpawnClass == nullptr)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (int32 Index = 0; Index < SpawnCount; ++Index)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		World->SpawnActor<AActor>(SpawnClass, GetRandomPointInVolume(), FRotator::ZeroRotator, SpawnParams);
	}
}

FVector ASpawnVolume::GetRandomPointInVolume() const
{
	const FVector Origin = SpawnBounds->Bounds.Origin;
	const FVector Extent = SpawnBounds->Bounds.BoxExtent;
	return UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent) + FVector(0.0f, 0.0f, SpawnHeightOffset);
}
