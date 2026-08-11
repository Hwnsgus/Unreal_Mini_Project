#include "TestActor.h"

#include "Test.h"

ATestActor::ATestActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATestActor::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTestModule, Display, TEXT("TestActor was spawned successfully by the Assing07 module."));
}
