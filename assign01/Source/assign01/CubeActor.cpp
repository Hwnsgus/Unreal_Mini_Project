#include "CubeActor.h"
#include "Engine/Engine.h" // AddOnScreenDebugMessage 사용을 위해 필요

ACubeActor::ACubeActor()
{
	PrimaryActorTick.bCanEverTick = false; // 이번 실습에선 틱이 필요 없음

	// 초기 좌표 설정 (0, 0, 50)
	CurrentLocation = FVector(0.0f, 0.0f, 50.0f);
}

void ACubeActor::BeginPlay()
{
	Super::BeginPlay();

	// 게임 시작 시 무작위 로직 10회 실행
	ExecuteRandomLogic();
}

void ACubeActor::Move()
{
	// 무작위 이동 거리 생성 (예: -100 ~ 100 사이)
	float RandX = FMath::RandRange(-100.0f, 100.0f);
	float RandY = FMath::RandRange(-100.0f, 100.0f);

	// 현재 좌표 업데이트
	CurrentLocation.X += RandX;
	CurrentLocation.Y += RandY;

	// 화면에 좌표 로그 출력 (AddOnScreenDebugMessage)
	// 매개변수: (Key, TimeToDisplay, Color, Message)
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan,
		FString::Printf(TEXT("Step % d - Move! New Location : X = % .2f, Y = % .2f, Z = % .2f"),
			EventCount, CurrentLocation.X, CurrentLocation.Y, CurrentLocation.Z));
}

void ACubeActor::Turn()
{
	// 무작위 회전 각도 생성 (0 ~ 360도)
	float RandYaw = FMath::RandRange(0.0f, 360.0f);

	// 화면에 회전 로그 출력
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow,
		FString::Printf(TEXT("Step % d - Turn! Rotation : %.2f degrees"), EventCount, RandYaw));
}

void ACubeActor::ExecuteRandomLogic()
{
	for (int32 i = 1; i <= 10; ++i)
	{
		EventCount = i;

		// 50% 확률로 이동하거나 회전함
		if (FMath::RandBool())
		{
			Move();
		}
		else
		{
			Turn();
		}
	}
}