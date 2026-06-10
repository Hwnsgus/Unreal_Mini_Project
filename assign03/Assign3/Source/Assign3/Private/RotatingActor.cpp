#include "RotatingActor.h"

ARotatingActor::ARotatingActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 기본값 설정
	RotationSpeed = FRotator(0.0f, 45.0f, 0.0f); // 초당 Yaw 방향 45도 회전
	LifetimeAfterStart = 10.0f; // 기본 10초 후 소멸
}

void ARotatingActor::BeginPlay()
{
	Super::BeginPlay();
    
	// 도전 과제: 타이머 세팅 (LifetimeAfterStart 초 후에 OnLifetimeExpired 함수 호출)
	if (LifetimeAfterStart > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			DestroyTimerHandle, 
			this, 
			&ARotatingActor::OnLifetimeExpired, 
			LifetimeAfterStart, 
			false
		);
	}
}

void ARotatingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 필수 기능: DeltaTime을 곱해 프레임 독립적인 회전 구현
	AddActorLocalRotation(RotationSpeed * DeltaTime);
}

void ARotatingActor::OnLifetimeExpired()
{
	// 타이머가 종료되면 발판 제거
	Destroy();
}