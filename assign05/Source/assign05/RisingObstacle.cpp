// Copyright Epic Games, Inc. All Rights Reserved.

#include "RisingObstacle.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

ARisingObstacle::ARisingObstacle()
{
	PrimaryActorTick.bCanEverTick = true;

	ObstacleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObstacleMesh"));
	SetRootComponent(ObstacleMesh);
	ObstacleMesh->SetMobility(EComponentMobility::Movable);
	ObstacleMesh->SetCollisionProfileName(TEXT("BlockAll"));
	ObstacleMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 0.5f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		ObstacleMesh->SetStaticMesh(CubeMesh.Object);
	}

	ActivationMessage = FText::FromString(TEXT("Obstacle activated!"));
}

void ARisingObstacle::BeginPlay()
{
	Super::BeginPlay();

	LoweredLocation = GetActorLocation();
	RaisedLocation = LoweredLocation + GetActorTransform().TransformVectorNoScale(LocalRiseOffset);
	bMovingToRaised = !bStartRaised;
	WaitTimer = ToggleInterval;

	if (bStartRaised)
	{
		SetActorLocation(RaisedLocation);
	}
}

void ARisingObstacle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bStartActive)
	{
		return;
	}

	if (WaitTimer > 0.0f)
	{
		WaitTimer -= DeltaSeconds;
		if (WaitTimer <= 0.0f && bMovingToRaised && bShowActivationMessage && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, ActivationMessageDuration, FColor::Red, ActivationMessage.ToString());
		}
		return;
	}

	MoveObstacle(DeltaSeconds);
}

void ARisingObstacle::MoveObstacle(float DeltaSeconds)
{
	const FVector Destination = bMovingToRaised ? RaisedLocation : LoweredLocation;
	const FVector CurrentLocation = GetActorLocation();
	const FVector Direction = (Destination - CurrentLocation).GetSafeNormal();
	const FVector NextLocation = CurrentLocation + Direction * MoveSpeed * DeltaSeconds;

	if (FVector::DistSquared(NextLocation, Destination) <= FMath::Square(MoveSpeed * DeltaSeconds))
	{
		SetActorLocation(Destination);
		ToggleMovementDirection();
		return;
	}

	SetActorLocation(NextLocation);
}

void ARisingObstacle::ToggleMovementDirection()
{
	bMovingToRaised = !bMovingToRaised;
	WaitTimer = ToggleInterval;
}
