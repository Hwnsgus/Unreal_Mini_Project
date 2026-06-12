// Copyright Epic Games, Inc. All Rights Reserved.

#include "PickupItemBase.h"

#include "Assign05GameMode.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

APickupItemBase::APickupItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(60.0f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(0.5f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
	}
}

void APickupItemBase::BeginPlay()
{
	Super::BeginPlay();

	Collision->OnComponentBeginOverlap.AddDynamic(this, &APickupItemBase::OnPickupOverlap);
}

void APickupItemBase::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* PickingPawn = Cast<APawn>(OtherActor);
	if (PickingPawn == nullptr)
	{
		return;
	}

	ApplyPickupEffect(PickingPawn);

	if (AAssign05GameMode* GameMode = Cast<AAssign05GameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->NotifyPickupCollected(ScoreValue);
	}

	if (bDestroyOnPickup)
	{
		Destroy();
	}
}

void APickupItemBase::ApplyPickupEffect_Implementation(APawn* PickingPawn)
{
}
