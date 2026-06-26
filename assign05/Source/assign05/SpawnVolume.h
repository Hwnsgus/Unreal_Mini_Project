// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnVolume.generated.h"

class UBoxComponent;

UCLASS()
class ASSIGN05_API ASpawnVolume : public AActor
{
	GENERATED_BODY()

public:
	ASpawnVolume();

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void SpawnWaveItems(int32 SpawnCount);

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void SpawnRequiredPickupItems(int32 SpawnCount);

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	FVector GetRandomPointInVolume() const;

protected:
	void SpawnItemsOfClass(TSubclassOf<AActor> SpawnClass, int32 SpawnCount);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> SpawnBounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TSubclassOf<AActor> RequiredPickupClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<TSubclassOf<AActor>> SpawnableClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float SpawnHeightOffset = 40.0f;
};
