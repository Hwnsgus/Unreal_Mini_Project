// Copyright Epic Games, Inc. All Rights Reserved.

#include "FallDeathTrigger.h"

#include "Assign05GameMode.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

AFallDeathTrigger::AFallDeathTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);
	TriggerBox->SetBoxExtent(FVector(2000.0f, 2000.0f, 100.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	GameOverMessage = FText::FromString(TEXT("YOU FELL"));
}

void AFallDeathTrigger::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AFallDeathTrigger::OnTriggerOverlap);
}

void AFallDeathTrigger::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* FallingPawn = Cast<APawn>(OtherActor);
	if (FallingPawn == nullptr)
	{
		return;
	}

	AAssign05GameMode* AssignGameMode = Cast<AAssign05GameMode>(UGameplayStatics::GetGameMode(this));
	if (AssignGameMode == nullptr)
	{
		return;
	}

	AssignGameMode->TriggerGameOverWithMessage(GameOverMessage);
}
