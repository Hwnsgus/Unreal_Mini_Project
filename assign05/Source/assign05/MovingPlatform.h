// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MovingPlatform.generated.h"

class UStaticMeshComponent;

UCLASS()
class ASSIGN05_API AMovingPlatform : public AActor
{
	GENERATED_BODY()

public:
	AMovingPlatform();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PlatformMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	FVector LocalTargetOffset = FVector(500.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform", meta = (ClampMin = "1.0"))
	float MoveSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform", meta = (ClampMin = "0.0"))
	float WaitTimeAtEnds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	bool bStartActive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	bool bReverseAtBeginPlay = false;

private:
	FVector StartLocation = FVector::ZeroVector;
	FVector TargetLocation = FVector::ZeroVector;
	bool bMovingToTarget = true;
	float WaitTimer = 0.0f;
};
