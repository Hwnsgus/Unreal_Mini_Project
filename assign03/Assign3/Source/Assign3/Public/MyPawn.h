#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "MyPawn.generated.h"

// 앞선 질문에서 정리한 대로 Public/Private 구조를 지킵니다.
class UCapsuleComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class ASSIGN3_API AMyPawn : public APawn
{
	GENERATED_BODY()

public:
	AMyPawn();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// --- 필수 컴포넌트 선언 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyPawn|Components")
	UCapsuleComponent* CapsuleComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyPawn|Components")
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyPawn|Components")
	UStaticMeshComponent* UFOStaticMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyPawn|Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyPawn|Components")
	UCameraComponent* CameraComp;
	
	// --- Enhanced Input 리플렉션 변수 (4개 액션세팅) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyPawn|Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyPawn|Input", meta = (DisplayName = "01_Move Action (WASD)"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyPawn|Input", meta = (DisplayName = "02_Look Action (Mouse)"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyPawn|Input", meta = (DisplayName = "03_Fly Up Down Action (Space/Shift)"))
	UInputAction* FlyUpDownAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyPawn|Input", meta = (DisplayName = "04_Roll Action (Q/E)"))
	UInputAction* RollAction;

	
	// --- 필수 및 도전 속성 변수 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyPawn|Movement")
	float MoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyPawn|Movement")
	float RotateSpeed;

	// 도전 과제: 중력 및 공중 제어용 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyPawn|Movement")
	bool bUseArtificialGravity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyPawn|Movement")
	float GravityZ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyPawn|Movement")
	float AirControlModifier; 

private:
	FVector2D MovementInput;
	FVector2D LookInput;
	float FlyUpDownInput;
	float RollInput;

	// 물리 시뮬레이션 없이 수식으로 연산할 중력 낙하 속도
	float VerticalVelocity;
	bool bIsGrounded;

	// 내부 함수들
	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnFlyUpDown(const FInputActionValue& Value);
	void OnRoll(const FInputActionValue& Value);
	void HandleMovement(float DeltaTime);
	void HandleRotation(float DeltaTime);
	void CheckGrounded();
};
