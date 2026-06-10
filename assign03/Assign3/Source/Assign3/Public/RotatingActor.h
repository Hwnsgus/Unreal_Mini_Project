#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RotatingActor.generated.h"

UCLASS()
class ASSIGN3_API ARotatingActor : public AActor
{
	GENERATED_BODY()
    
public:    
	ARotatingActor();

protected:
	virtual void BeginPlay() override;

public:    
	virtual void Tick(float DeltaTime) override;

	// --- 필수 기능: 리플렉션 적용 및 에디터 노출 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IdolPlatform|Required")
	FRotator RotationSpeed;

	// --- 도전 기능: 타이머 시스템 활용 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IdolPlatform|Challenge")
	float LifetimeAfterStart; // 몇 초 뒤에 사라질 것인가?

	FTimerHandle DestroyTimerHandle;

	// 타이머에 의해 호출될 함수 (UFUNCTION 필수)
	UFUNCTION()
	void OnLifetimeExpired();
};