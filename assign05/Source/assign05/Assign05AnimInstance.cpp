// Copyright Epic Games, Inc. All Rights Reserved.

#include "Assign05AnimInstance.h"

#include "Assign05Character.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

void UAssign05AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningPawn = TryGetPawnOwner();
}

void UAssign05AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwningPawn == nullptr)
	{
		OwningPawn = TryGetPawnOwner();
	}

	if (OwningPawn == nullptr)
	{
		return;
	}

	const FVector Velocity = OwningPawn->GetVelocity();
	const FVector HorizontalVelocity(Velocity.X, Velocity.Y, 0.0f);
	Speed = HorizontalVelocity.Size();
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, OwningPawn->GetActorRotation());

	if (const ACharacter* Character = Cast<ACharacter>(OwningPawn))
	{
		if (const UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			bIsInAir = Movement->IsFalling();
			bIsAccelerating = Movement->GetCurrentAcceleration().SizeSquared() > 0.0f;
		}
	}

	if (const AAssign05Character* AssignCharacter = Cast<AAssign05Character>(OwningPawn))
	{
		bIsSprinting = AssignCharacter->IsSprinting();
	}
}
