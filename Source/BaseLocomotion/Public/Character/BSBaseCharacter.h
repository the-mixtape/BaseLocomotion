// Mixtape's Base Locomotion All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Library/BSCharacterEnumLibrary.h"
#include "Library/BSCharacterStructLibrary.h"
#include "BSBaseCharacter.generated.h"


class UBSCharacterMovementComponent;
class UBSPlayerCameraBehavior;


UCLASS()
class BASELOCOMOTION_API ABSBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABSBaseCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual auto OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) -> void override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
#pragma region Input
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "BaseLocomotion|Input")
	void ForwardMovementAction(float Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "BaseLocomotion|Input")
	void RightMovementAction(float Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "BaseLocomotion|Input")
	void CameraUpAction(float Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "BaseLocomotion|Input")
	void CameraRightAction(float Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "BaseLocomotion|Input")
	void JumpAction(bool bValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "BaseLocomotion|Input")
	void RunAction(bool bValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "BaseLocomotion|Input")
	void AimAction(bool bValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "BaseLocomotion|Input")
	void StanceAction();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "BaseLocomotion|Input")
	void VelocityDirectionAction();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "BaseLocomotion|Input")
	void LookingDirectionAction();
	
	UFUNCTION(BlueprintSetter, Category = "BaseLocomotion|Input")
	void SetDesiredRotationMode(EBSRotationMode NewRotMode);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "BaseLocomotion|Character States")
	void Server_SetDesiredRotationMode(EBSRotationMode NewRotMode);
	
	UFUNCTION(BlueprintPure, Category = "BaseLocomotion|Character States")
	EBSGait GetDesiredGait() const { return DesiredGait; }
	
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Character States")
	void SetDesiredGait(EBSGait NewGait);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "BaseLocomotion|Character States")
	void Server_SetDesiredGait(EBSGait NewGait);

	UFUNCTION(BlueprintSetter, Category = "BaseLocomotion|Input")
	void SetDesiredStance(EBSStance NewStance);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "BaseLocomotion|Input")
	void Server_SetDesiredStance(EBSStance NewStance);
	
#pragma endregion
	
#pragma region CharacterStates
public:
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Character States")
	void SetMovementState(EBSMovementState NewState, bool bForce = false);
	
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Character States")
	void SetGait(EBSGait NewGait, bool bForce = false);
	
	UFUNCTION(BlueprintGetter, Category = "BaseLocomotion|Character States")
	EBSGait GetGait() const { return Gait; }
	
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Character States")
	void SetStance(EBSStance NewStance, bool bForce = false);

	UFUNCTION(BlueprintGetter, Category = "BaseLocomotion|Character States")
	EBSStance GetStance() const { return Stance; }

	UFUNCTION(BlueprintGetter, Category = "BaseLocomotion|Character States")
	EBSRotationMode GetRotationMode() const { return RotationMode; }
	
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Character States")
	void SetRotationMode(EBSRotationMode NewRotationMode, bool bForce = false);
	
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "BaseLocomotion|Character States")
	void Server_SetRotationMode(EBSRotationMode NewRotationMode, bool bForce);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|State Values")
	EBSGait Gait = EBSGait::Walking;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseLocomotion|State Values")
	EBSStance Stance = EBSStance::Standing;

	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|State Values")
	EBSMovementState MovementState = EBSMovementState::None;

	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|State Values")
	EBSMovementState PrevMovementState = EBSMovementState::None;
	
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|State Values", ReplicatedUsing = OnRep_RotationMode)
	EBSRotationMode RotationMode = EBSRotationMode::LookingDirection;

protected:
	virtual void OnRotationModeChanged(EBSRotationMode PreviousRotationMode);
#pragma endregion
	
#pragma region Camera
public:
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Camera System")
	void SetCameraBehavior(UBSPlayerCameraBehavior* CamBeh) { CameraBehavior = CamBeh; }

	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Camera System")
	virtual FTransform GetThirdPersonPivotTarget();
	
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Camera System")
	virtual ECollisionChannel GetThirdPersonTraceParams(FVector& TraceOrigin, float& TraceRadius);
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Camera")
	TObjectPtr<UBSPlayerCameraBehavior> CameraBehavior;
#pragma endregion

#pragma region Replication
protected:
	UFUNCTION(Category = "BaseLocomotion|Replication")
	void OnRep_RotationMode(EBSRotationMode PrevRotMode);
#pragma endregion

#pragma region Utility
public:
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Utility")
	float GetAnimCurveValue(FName CurveName) const;
#pragma endregion
	
#pragma region Utils 
protected:
	void SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed, float DeltaTime);
	void SetMovementModel();
	void ForceUpdateCharacterState();
	float CalculateGroundedRotationRate() const;
	void LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime);
#pragma endregion

private:
	virtual void OnGaitChanged(EBSGait PreviousGait);
	virtual void OnStanceChanged(EBSStance PreviousStance);
	virtual void OnMovementStateChanged(EBSMovementState PreviousState);
	
	void SetEssentialValues(float DeltaTime);
	void UpdateCharacterMovement();
	void UpdateGroundedRotation(float DeltaTime);
	void UpdateInAirRotation(float DeltaTime);
	
protected:
	/* Custom movement component*/
	UPROPERTY()
	TObjectPtr<UBSCharacterMovementComponent> MyCharacterMovementComponent;

	/** Input */
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "BaseLocomotion|Input")
	EBSRotationMode DesiredRotationMode = EBSRotationMode::LookingDirection;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "BaseLocomotion|Input")
	EBSGait DesiredGait = EBSGait::Walking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "BaseLocomotion|Input")
	EBSStance DesiredStance = EBSStance::Standing;
	
	UPROPERTY(EditDefaultsOnly, Category = "BaseLocomotion|Input", BlueprintReadOnly)
	float LookUpDownRate = 1.25f;

	UPROPERTY(EditDefaultsOnly, Category = "BaseLocomotion|Input", BlueprintReadOnly)
	float LookLeftRightRate = 1.25f;
	
	/** Movement System */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseLocomotion|Movement System")
	FDataTableRowHandle MovementModel;
	
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Movement System")
	FBSMovementStateSettings MovementData;
	
	/** Rotation System */
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Rotation System")
	FRotator TargetRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Rotation System")
	FRotator InAirRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Rotation System")
	float YawOffset = 0.0f;

	/** Essential Information */
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Essential Information")
	FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Essential Information")
	bool bIsMoving = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Essential Information")
	bool bHasMovementInput = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Essential Information")
	FRotator LastVelocityRotation;

	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Essential Information")
	FRotator LastMovementInputRotation;
	
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Essential Information")
	float Speed = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Essential Information")
	float MovementInputAmount = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Essential Information")
	float AimYawRate = 0.0f;
	
	/** Replicated Essential Information*/
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Essential Information")
	float EasedMaxAcceleration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "BaseLocomotion|Essential Information")
	FVector ReplicatedCurrentAcceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "BaseLocomotion|Essential Information")
	FRotator ReplicatedControlRotation = FRotator::ZeroRotator;

public:
	UFUNCTION(BlueprintPure, Category = "BaseLocomotion|Essential Information")
	FRotator GetAimingRotation() const { return AimingRotation; }
	
	UFUNCTION(BlueprintGetter, Category = "BaseLocomotion|Essential Information")
	float GetAimYawRate() const { return AimYawRate; }

public:
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Movement System")
	FBSMovementSettings GetTargetMovementSettings() const;
	
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Movement System")
	EBSGait GetAllowedGait() const;
	
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Movement System")
	EBSGait GetActualGait(EBSGait AllowedGait) const;
	
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Movement System")
	bool CanRun() const;

protected:
	/** Cached Variables */
	FVector PreviousVelocity = FVector::ZeroVector;
	float PreviousAimYaw = 0.0f;
	
	/* Smooth out aiming by interping control rotation*/
	FRotator AimingRotation = FRotator::ZeroRotator;
};
