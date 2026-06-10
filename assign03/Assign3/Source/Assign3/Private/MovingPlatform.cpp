#include "MovingPlatform.h"


AMovingPlatform::AMovingPlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	// 기본값 세팅
	MoveSpeed = FVector(0.0f, 200.0f, 0.0f); // Y축 기준 초당 200 속도
	MaxRange = 500.0f; // 500센티미터 이동 후 왕복
	MoveDirection = 1;
	bRandomizeOnSpawn = false;
}

void AMovingPlatform::BeginPlay()
{
	Super::BeginPlay();
    
	// 시작 위치 기억 및 제한 기준점으로 사용
	StartLocation = GetActorLocation();

	// 도전 과제: 랜덤 속성 부여
	if (bRandomizeOnSpawn)
	{
		MoveSpeed.Y = FMath::RandRange(100.0f, 400.0f);
		MaxRange = FMath::RandRange(300.0f, 800.0f);
	}
}

void AMovingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 현재 위치와 시작 위치 사이의 거리 계산
	FVector CurrentLocation = GetActorLocation();
	float DistanceMoved = FVector::Dist(StartLocation, CurrentLocation);

	// 최대 범위를 벗어나면 방향 전환
	if (DistanceMoved >= MaxRange)
	{
		// 범위를 초과해서 튕겨 나가지 않도록 위치 보정
		FVector MoveDirVector = MoveSpeed.GetSafeNormal();
		CurrentLocation = StartLocation + (MoveDirVector * MaxRange * MoveDirection);
		SetActorLocation(CurrentLocation);

		// 방향 뒤집기
		MoveDirection *= -1;
	}

	// 프레임 독립적 이동 처리 (DeltaTime 활용)
	FVector NewLocation = GetActorLocation() + (MoveSpeed * MoveDirection * DeltaTime);
	SetActorLocation(NewLocation);
}