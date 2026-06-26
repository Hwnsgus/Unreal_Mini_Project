// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Assign05Character.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UTextBlock;
class UUserWidget;
class UWidget;
class UWidgetComponent;

UCLASS()
class ASSIGN05_API AAssign05Character : public ACharacter
{
	GENERATED_BODY()

public:
	AAssign05Character();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsSprinting() const { return bWantsToSprint; }

	float GetHealth() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	void AddHealth(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Debuff")
	void ApplyCameraReverseDebuff(float Duration);
	
protected:
	virtual void BeginPlay() override;
	
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void StartSprint();
	void StopSprint();
	void EndCameraReverseDebuff();

	void ApplyMovementSpeed();
	void ConfigureHPWidgetComponent();
	void UpdateHPWidgetTransform();
	void RefreshHPWidget();
	void CenterHPTextWidget(UWidget* TextWidget) const;
	bool SetHPTextOnWidget(UUserWidget* HPWidget, const FText& HPText) const;
	
	void OnDeath();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float Health = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health|UI")
	TObjectPtr<UWidgetComponent> HPWidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|UI")
	TSubclassOf<UUserWidget> HPWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|UI")
	FVector HPWidgetOffset = FVector(0.0f, 0.0f, 135.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|UI")
	FVector2D HPWidgetDrawSize = FVector2D(180.0f, 50.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|UI", meta = (ClampMin = "0.01"))
	float HPWidgetWorldScale = 0.35f;

	virtual float TakeDamage(float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,AActor* DamageCauser) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float JumpVelocity = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AirControlAmount = 0.35f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bWantsToSprint = false;

	UPROPERTY(BlueprintReadOnly, Category = "Debuff")
	bool bCameraViewReversed = false;

	FRotator DefaultFollowCameraRelativeRotation = FRotator::ZeroRotator;
	FTimerHandle CameraReverseTimerHandle;
};
