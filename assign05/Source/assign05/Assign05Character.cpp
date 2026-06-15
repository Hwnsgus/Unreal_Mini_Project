// Copyright Epic Games, Inc. All Rights Reserved.

#include "Assign05Character.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

AAssign05Character::AAssign05Character()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	Movement->JumpZVelocity = JumpVelocity;
	Movement->AirControl = AirControlAmount;
	Movement->MaxWalkSpeed = WalkSpeed;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 450.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void AAssign05Character::BeginPlay()
{
	Super::BeginPlay();

	Health = FMath::Clamp(Health, 0.0f, MaxHealth);
	ApplyMovementSpeed();
}

void AAssign05Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AAssign05Character::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AAssign05Character::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AAssign05Character::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AAssign05Character::LookUp);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &AAssign05Character::StartSprint);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &AAssign05Character::StopSprint);
}

void AAssign05Character::MoveForward(float Value)
{
	if (Controller == nullptr || FMath::IsNearlyZero(Value))
	{
		return;
	}

	const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(Direction, Value);
}

void AAssign05Character::MoveRight(float Value)
{
	if (Controller == nullptr || FMath::IsNearlyZero(Value))
	{
		return;
	}

	const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(Direction, Value);
}

void AAssign05Character::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AAssign05Character::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AAssign05Character::StartSprint()
{
	bWantsToSprint = true;
	ApplyMovementSpeed();
}

void AAssign05Character::StopSprint()
{
	bWantsToSprint = false;
	ApplyMovementSpeed();
}

void AAssign05Character::ApplyMovementSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = bWantsToSprint ? SprintSpeed : WalkSpeed;
	GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	GetCharacterMovement()->AirControl = AirControlAmount;
}

float AAssign05Character::GetHealth() const
{
	return Health;
}

void AAssign05Character::AddHealth(float Amount)
{
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	UE_LOG(LogTemp, Warning, TEXT("Health: %f"), Health);
}

float AAssign05Character::TakeDamage(float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount,DamageEvent,EventInstigator,DamageCauser);
	
	const float ActualDamage = FMath::Max(0.0f, DamageAmount);
	Health = FMath::Clamp(Health - ActualDamage,0.0f, MaxHealth);
	UE_LOG(LogTemp, Warning, TEXT("Health: %f"), Health);
	
	if (Health <= 0.0f)
	{
		OnDeath();
	}
	
	return ActualDamage;
}

void AAssign05Character::OnDeath()
{
	//게임 종료 로직
}
