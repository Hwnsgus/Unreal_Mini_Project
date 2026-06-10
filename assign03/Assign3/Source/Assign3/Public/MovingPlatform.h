#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MovingPlatform.generated.h"

UCLASS()
class ASSIGN3_API AMovingPlatform : public AActor
{
	GENERATED_BODY()
    
public:    
	AMovingPlatform();

protected:
	virtual void BeginPlay() override;

public:    
	virtual void Tick(float DeltaTime) override;

	// --- 필수 기능: 이동 관련 UPROPERTY ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IdolPlatform|Required")
	FVector MoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IdolPlatform|Required")
	float MaxRange;

	// --- 도전 기능: 시작 시 무작위 속성 부여 여부 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IdolPlatform|Challenge")
	bool bRandomizeOnSpawn;

private:
	FVector StartLocation;
	int32 MoveDirection; 
};