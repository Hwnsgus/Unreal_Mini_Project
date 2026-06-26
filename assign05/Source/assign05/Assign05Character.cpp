// Copyright Epic Games, Inc. All Rights Reserved.

#include "Assign05Character.h"

#include "Assign05GameMode.h"
#include "Camera/PlayerCameraManager.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CapsuleComponent.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Components/WidgetComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

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

	HPWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPWidget"));
	HPWidgetComponent->SetupAttachment(RootComponent);
	HPWidgetComponent->SetRelativeLocation(HPWidgetOffset);
	HPWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	HPWidgetComponent->SetDrawSize(HPWidgetDrawSize);
	HPWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
	HPWidgetComponent->SetTwoSided(true);
	HPWidgetComponent->SetRelativeScale3D(FVector(HPWidgetWorldScale));
	HPWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FClassFinder<UUserWidget> HPWidgetFinder(TEXT("/Game/Assign05/UI/WBP_HP"));
	if (HPWidgetFinder.Succeeded())
	{
		HPWidgetClass = HPWidgetFinder.Class;
		HPWidgetComponent->SetWidgetClass(HPWidgetClass);
	}
}

void AAssign05Character::BeginPlay()
{
	Super::BeginPlay();

	Health = FMath::Clamp(Health, 0.0f, MaxHealth);
	if (FollowCamera)
	{
		DefaultFollowCameraRelativeRotation = FollowCamera->GetRelativeRotation();
	}
	ConfigureHPWidgetComponent();
	UpdateHPWidgetTransform();
	RefreshHPWidget();
	ApplyMovementSpeed();
}

void AAssign05Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateHPWidgetTransform();
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
	const float InputMultiplier = bCameraViewReversed ? -1.0f : 1.0f;
	AddControllerYawInput(Value * InputMultiplier);
}

void AAssign05Character::LookUp(float Value)
{
	const float InputMultiplier = bCameraViewReversed ? -1.0f : 1.0f;
	AddControllerPitchInput(Value * InputMultiplier);
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

void AAssign05Character::ConfigureHPWidgetComponent()
{
	if (HPWidgetComponent == nullptr)
	{
		return;
	}

	if (HPWidgetClass)
	{
		HPWidgetComponent->SetWidgetClass(HPWidgetClass);
	}

	HPWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	HPWidgetComponent->SetDrawSize(HPWidgetDrawSize);
	HPWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
	HPWidgetComponent->SetTwoSided(true);
	HPWidgetComponent->SetWorldScale3D(FVector(HPWidgetWorldScale));
	HPWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAssign05Character::UpdateHPWidgetTransform()
{
	if (HPWidgetComponent == nullptr)
	{
		return;
	}

	const FVector WidgetLocation = GetActorLocation() + HPWidgetOffset;
	HPWidgetComponent->SetWorldLocation(WidgetLocation);

	if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		const FVector DirectionToCamera = CameraManager->GetCameraLocation() - WidgetLocation;
		if (!DirectionToCamera.IsNearlyZero())
		{
			HPWidgetComponent->SetWorldRotation(DirectionToCamera.Rotation());
		}
	}
}

float AAssign05Character::GetHealth() const
{
	return Health;
}

void AAssign05Character::AddHealth(float Amount)
{
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	RefreshHPWidget();
	UE_LOG(LogTemp, Warning, TEXT("Health: %f"), Health);
}

void AAssign05Character::ApplyCameraReverseDebuff(float Duration)
{
	if (Duration <= 0.0f)
	{
		return;
	}

	bCameraViewReversed = true;

	if (FollowCamera)
	{
		const FRotator ReversedRotation(
			DefaultFollowCameraRelativeRotation.Pitch,
			DefaultFollowCameraRelativeRotation.Yaw,
			DefaultFollowCameraRelativeRotation.Roll + 180.0f);
		FollowCamera->SetRelativeRotation(ReversedRotation);
	}

	GetWorldTimerManager().ClearTimer(CameraReverseTimerHandle);
	GetWorldTimerManager().SetTimer(CameraReverseTimerHandle, this, &AAssign05Character::EndCameraReverseDebuff, Duration, false);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, TEXT("Camera reversed!"));
	}
}

void AAssign05Character::EndCameraReverseDebuff()
{
	bCameraViewReversed = false;

	if (FollowCamera)
	{
		FollowCamera->SetRelativeRotation(DefaultFollowCameraRelativeRotation);
	}
}

float AAssign05Character::TakeDamage(float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount,DamageEvent,EventInstigator,DamageCauser);
	
	const float ActualDamage = FMath::Max(0.0f, DamageAmount);
	Health = FMath::Clamp(Health - ActualDamage,0.0f, MaxHealth);
	RefreshHPWidget();
	UE_LOG(LogTemp, Warning, TEXT("Health: %f"), Health);
	
	if (Health <= 0.0f)
	{
		OnDeath();
	}
	
	return ActualDamage;
}

void AAssign05Character::OnDeath()
{
	if (AAssign05GameMode* AssignGameMode = Cast<AAssign05GameMode>(UGameplayStatics::GetGameMode(this)))
	{
		AssignGameMode->TriggerGameOverWithMessage(FText::FromString(TEXT("HP ZERO")));
	}

	//게임 종료 로직
}
void AAssign05Character::RefreshHPWidget()
{
	if (HPWidgetComponent == nullptr)
	{
		return;
	}

	UUserWidget* HPWidget = HPWidgetComponent->GetUserWidgetObject();
	if (HPWidget == nullptr)
	{
		return;
	}

	const int32 CurrentHP = FMath::RoundToInt(Health);
	const int32 MaxHP = FMath::RoundToInt(MaxHealth);
	const FText HPText = FText::FromString(FString::Printf(TEXT("%d/%d"), CurrentHP, MaxHP));
	SetHPTextOnWidget(HPWidget, HPText);
}

void AAssign05Character::CenterHPTextWidget(UWidget* TextWidget) const
{
	if (TextWidget == nullptr)
	{
		return;
	}

	TextWidget->SetVisibility(ESlateVisibility::HitTestInvisible);

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(TextWidget->Slot))
	{
		CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetPosition(FVector2D::ZeroVector);
		CanvasSlot->SetSize(HPWidgetDrawSize);
	}
}

bool AAssign05Character::SetHPTextOnWidget(UUserWidget* HPWidget, const FText& HPText) const
{
	if (HPWidget == nullptr)
	{
		return false;
	}

	static const FName HPTextNames[] =
	{
		TEXT("OverHeadHP"),
		TEXT("OverheadHP"),
		TEXT("HPText"),
		TEXT("HealthText"),
		TEXT("HealthValueText"),
		TEXT("HPValueText"),
		TEXT("HP"),
		TEXT("HP_Text"),
		TEXT("TextBlock"),
		TEXT("TextBlock_0"),
		TEXT("TextBlock_1"),
		TEXT("Text")
	};

	for (const FName& WidgetName : HPTextNames)
	{
		if (UTextBlock* TextBlock = Cast<UTextBlock>(HPWidget->GetWidgetFromName(WidgetName)))
		{
			TextBlock->SetText(HPText);
			CenterHPTextWidget(TextBlock);
			return true;
		}

		if (UEditableTextBox* TextBox = Cast<UEditableTextBox>(HPWidget->GetWidgetFromName(WidgetName)))
		{
			TextBox->SetText(HPText);
			TextBox->SetIsReadOnly(true);
			CenterHPTextWidget(TextBox);
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("WBP_HP text was not found. Name a TextBlock 'HPText' or 'HealthText'."));
	return false;
}
