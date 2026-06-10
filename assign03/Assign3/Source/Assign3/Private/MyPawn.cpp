#include "MyPawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"


AMyPawn::AMyPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// 1. 필수 기능: 컴포넌트 생성 및 계층 구조(Root) 설정
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	SetRootComponent(CapsuleComp);

	// 물리 시뮬레이션 끄기 (Simulate Physics = false 필수 조건 충족)
	CapsuleComp->SetSimulatePhysics(false);
	CapsuleComp->SetEnableGravity(false);
	CapsuleComp->SetCollisionProfileName(TEXT("Pawn"));

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetEnableGravity(false);

	UFOStaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UFOStaticMeshComp"));
	UFOStaticMeshComp->SetupAttachment(RootComponent);
	UFOStaticMeshComp->SetSimulatePhysics(false);
	UFOStaticMeshComp->SetEnableGravity(false);
	
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 400.0f;
	// 마우스 회전 제어 수식을 직접 사용하므로 폰 회전 동기화 비활성화
	SpringArmComp->bUsePawnControlRotation = false; 

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);

	// 2. 변수 기본 설정값 정의
	MoveSpeed = 600.0f;
	RotateSpeed = 45.0f;
	bUseArtificialGravity = false;
	GravityZ = -980.0f; // 도전 과제: 인공 중력 가속도 cm/s^2
	AirControlModifier = 1.0f; // UFO 기본 동작은 공중에서도 같은 조작감 유지
	
	VerticalVelocity = 0.0f;
	bIsGrounded = false;
	FlyUpDownInput = 0.0f;
	RollInput = 0.0f;
}

void AMyPawn::BeginPlay()
{
	Super::BeginPlay();

	// Enhanced Input Subsystem 등록 연동
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void AMyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 도전 과제: 지면 충돌 체크 및 착지 처리 함수 호출
	if (bUseArtificialGravity)
	{
		CheckGrounded();
	}
	else
	{
		bIsGrounded = false;
		VerticalVelocity = 0.0f;
	}

	// 프레임 독립적인 움직임 연산 호출 (DeltaTime 전달)
	HandleMovement(DeltaTime);
	HandleRotation(DeltaTime);
}

void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Enhanced Input 액션 바인딩 연결
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyPawn::OnMove);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AMyPawn::OnMove);
		}
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyPawn::OnLook);
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Completed, this, &AMyPawn::OnLook);
		}
		if (FlyUpDownAction)
		{
			EnhancedInputComponent->BindAction(FlyUpDownAction, ETriggerEvent::Triggered, this, &AMyPawn::OnFlyUpDown);
			EnhancedInputComponent->BindAction(FlyUpDownAction, ETriggerEvent::Completed, this, &AMyPawn::OnFlyUpDown);
		}
		if (RollAction)
		{
			EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &AMyPawn::OnRoll);
			EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Completed, this, &AMyPawn::OnRoll);
		}
	}
}

void AMyPawn::OnMove(const FInputActionValue& Value)
{
	MovementInput = Value.Get<FVector2D>();
}

void AMyPawn::OnLook(const FInputActionValue& Value)
{
	LookInput = Value.Get<FVector2D>();
}

void AMyPawn::OnFlyUpDown(const FInputActionValue& Value)
{
	FlyUpDownInput = Value.Get<float>();
}

void AMyPawn::OnRoll(const FInputActionValue& Value)
{
	RollInput = Value.Get<float>();
}

void AMyPawn::HandleMovement(float DeltaTime)
{
	// 에어 컨트롤 로직 분기 적용
	float CurrentSpeed = MoveSpeed;
	if (bUseArtificialGravity && !bIsGrounded)
	{
		CurrentSpeed *= AirControlModifier; // 공중에 떠 있을 땐 속도 디버프 적용
	}

	// 폰의 Forward/Right 방향 벡터 기준으로 로컬 좌표계 연산
	FVector ForwardDirection = GetActorForwardVector();
	FVector RightDirection = GetActorRightVector();
	FVector UpDirection = GetActorUpVector();

	// 6DOF 도전 과제: 폰의 로컬 Forward/Right/Up 방향 기준 이동 벡터 합성
	FVector MapMovement =
		(ForwardDirection * MovementInput.Y) +
		(RightDirection * MovementInput.X) +
		(UpDirection * FlyUpDownInput);
	MapMovement.Normalize();

	FVector NewOffset = MapMovement * CurrentSpeed * DeltaTime;

	// 도전 과제: 인공 중력 가속도 연산 추가 (수학적 제어)
	if (bUseArtificialGravity && !bIsGrounded)
	{
		VerticalVelocity += GravityZ * DeltaTime;
	}
	else if (bUseArtificialGravity)
	{
		VerticalVelocity = 0.0f; // 지면에 서 있다면 하강 속도는 0
	}
	NewOffset.Z += VerticalVelocity * DeltaTime;

	// 최종 트랜스폼 오프셋 변동량 적용
	AddActorLocalOffset(NewOffset, true); // Sweep을 true로 주어 장애물 충돌 보장
}

void AMyPawn::HandleRotation(float DeltaTime)
{
	if (LookInput.IsNearlyZero() && FMath::IsNearlyZero(RollInput))
	{
		return;
	}

	// 필수/도전 조건: AddControllerYawInput 등을 쓰지 않고 Yaw/Pitch/Roll을 직접 계산
	float YawRotation = LookInput.X * RotateSpeed * DeltaTime;
	float PitchRotation = LookInput.Y * RotateSpeed * DeltaTime;
	float RollRotation = RollInput * RotateSpeed * DeltaTime;

	AddActorLocalRotation(FRotator(PitchRotation, YawRotation, RollRotation));
}

void AMyPawn::CheckGrounded()
{
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FHitResult HitResult;
	FVector Start = GetActorLocation();
	// 캡슐 절반 높이보다 살짝 아래까지 트레이스 선을 뻗음
	float TraceDistance = CapsuleComp->GetScaledCapsuleHalfHeight() + 5.0f;
	FVector End = Start + (FVector::UpVector * -TraceDistance);

	// 도전 과제: LineTrace를 사용하여 실시간 지면 레이캐스트 확인
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

	bIsGrounded = bHit;
}
