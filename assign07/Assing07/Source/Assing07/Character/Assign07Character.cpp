// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/Assign07Character.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "TestActor.h"

AAssign07Character::AAssign07Character()
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->TargetArmLength = 400.0f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->bUsePawnControlRotation = false;
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
}

void AAssign07Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(
		MoveAction, ETriggerEvent::Triggered, this, &ThisClass::HandleMoveInput);
	EnhancedInputComponent->BindAction(
		LookAction, ETriggerEvent::Triggered, this, &ThisClass::HandleLookInput);
	EnhancedInputComponent->BindAction(
		JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
	EnhancedInputComponent->BindAction(
		JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
}

void AAssign07Character::BeginPlay()
{
	Super::BeginPlay();

	// 액터 생성은 서버에서만 수행해 멀티플레이 PIE에서 중복 생성을 방지합니다.
	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = this;
			SpawnParameters.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			const FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 150.0f;
			World->SpawnActor<ATestActor>(
				ATestActor::StaticClass(), SpawnLocation, GetActorRotation(), SpawnParameters);
		}
	}

	if (IsLocallyControlled())
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		checkf(IsValid(PlayerController), TEXT("PlayerController is invalid."));

		UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		checkf(IsValid(InputSubsystem), TEXT("EnhancedInputLocalPlayerSubsystem is invalid."));

		InputSubsystem->AddMappingContext(InputMappingContext, 0);
	}
}

void AAssign07Character::HandleMoveInput(const FInputActionValue& InValue)
{
	if (!IsValid(Controller))
	{
		UE_LOG(LogTemp, Error, TEXT("Controller is invalid."));
		return;
	}

	const FVector2D MovementVector = InValue.Get<FVector2D>();
	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator ControlYawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection =
		FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection =
		FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void AAssign07Character::HandleLookInput(const FInputActionValue& InValue)
{
	if (!IsValid(Controller))
	{
		UE_LOG(LogTemp, Error, TEXT("Controller is invalid."));
		return;
	}

	const FVector2D LookVector = InValue.Get<FVector2D>();

	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
}
