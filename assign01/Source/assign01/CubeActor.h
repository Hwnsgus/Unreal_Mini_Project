#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CubeActor.generated.h"

UCLASS()
class ASSIGN01_API ACubeActor : public AActor
{
	GENERATED_BODY()

public:
	ACubeActor();

protected:
	virtual void BeginPlay() override;

public:
	// 이동 로직 함수
	void Move();

	// 회전 로직 함수
	void Turn();

	// 현재 위치를 저장할 변수 (시작 위치 0,0,50 반영)
	FVector CurrentLocation;

	// 무작위 이동/회전 10회 실행 함수
	void ExecuteRandomLogic();

	// 이동 횟수 카운트
	int32 EventCount = 0;
};