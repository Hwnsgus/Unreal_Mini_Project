// Copyright Epic Games, Inc. All Rights Reserved.

#include "MovingPlatform.h"

#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AMovingPlatform::AMovingPlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	SetRootComponent(PlatformMesh);
	PlatformMesh->SetMobility(EComponentMobility::Movable);
	PlatformMesh->SetCollisionProfileName(TEXT("BlockAll"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		PlatformMesh->SetStaticMesh(CubeMesh.Object);
		PlatformMesh->SetRelativeScale3D(FVector(3.0f, 3.0f, 0.25f));
	}
}

void AMovingPlatform::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
	TargetLocation = StartLocation + GetActorTransform().TransformVectorNoScale(LocalTargetOffset);
	bMovingToTarget = !bReverseAtBeginPlay;
}

void AMovingPlatform::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bStartActive)
	{
		return;
	}

	if (WaitTimer > 0.0f)
	{
		WaitTimer -= DeltaSeconds;
		return;
	}

	const FVector Destination = bMovingToTarget ? TargetLocation : StartLocation;
	const FVector CurrentLocation = GetActorLocation();
	const FVector Direction = (Destination - CurrentLocation).GetSafeNormal();
	const FVector NextLocation = CurrentLocation + Direction * MoveSpeed * DeltaSeconds;

	if (FVector::DistSquared(NextLocation, Destination) <= FMath::Square(MoveSpeed * DeltaSeconds))
	{
		SetActorLocation(Destination);
		bMovingToTarget = !bMovingToTarget;
		WaitTimer = WaitTimeAtEnds;
		return;
	}

	SetActorLocation(NextLocation);
}
