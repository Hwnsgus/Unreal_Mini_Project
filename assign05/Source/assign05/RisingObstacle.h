// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RisingObstacle.generated.h"

class UStaticMeshComponent;

UCLASS()
class ASSIGN05_API ARisingObstacle : public AActor
{
	GENERATED_BODY()

public:
	ARisingObstacle();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ObstacleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle")
	FVector LocalRiseOffset = FVector(0.0f, 0.0f, 180.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle", meta = (ClampMin = "0.1"))
	float ToggleInterval = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle", meta = (ClampMin = "1.0"))
	float MoveSpeed = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle")
	bool bStartRaised = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle")
	bool bStartActive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	bool bShowActivationMessage = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FText ActivationMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (ClampMin = "0.1"))
	float ActivationMessageDuration = 2.0f;

private:
	void MoveObstacle(float DeltaSeconds);
	void ToggleMovementDirection();

private:
	FVector LoweredLocation = FVector::ZeroVector;
	FVector RaisedLocation = FVector::ZeroVector;
	bool bMovingToRaised = true;
	float WaitTimer = 0.0f;
};
