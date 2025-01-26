// Mixtape's Base Locomotion All Rights Reserved.


#include "Character/BSBaseCharacter.h"

#include "Character/BSCharacterMovementComponent.h"
#include "Character/BSPlayerCameraBehavior.h"
#include "Kismet/KismetMathLibrary.h"
#include "Library/BSCharacterStructLibrary.h"
#include "Net/UnrealNetwork.h"

namespace
{
	const FName NAME_YawOffset(TEXT("YawOffset"));
	const FName NAME_RotationAmount(TEXT("RotationAmount"));
}

ABSBaseCharacter::ABSBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBSCharacterMovementComponent>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = 0;
	bReplicates = true;
	SetReplicatingMovement(true);
}

void ABSBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetMesh()->AddTickPrerequisiteActor(this);
	SetMovementModel();
	ForceUpdateCharacterState();

	if (Stance == EBSStance::Standing)
	{
		UnCrouch();
	}
	else if (Stance == EBSStance::Crouching)
	{
		Crouch();
	}

	// Set default rotation values.
	TargetRotation = GetActorRotation();
	LastVelocityRotation = TargetRotation;
	LastMovementInputRotation = TargetRotation;

	if (GetMesh()->GetAnimInstance() && GetLocalRole() == ROLE_SimulatedProxy)
	{
		GetMesh()->GetAnimInstance()->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
	}

	MyCharacterMovementComponent->SetMovementSettings(GetTargetMovementSettings());
}

void ABSBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Set required values
	SetEssentialValues(DeltaTime);

	if (MovementState == EBSMovementState::Grounded)
	{
		UpdateCharacterMovement();
		UpdateGroundedRotation(DeltaTime);
	}
	else if (MovementState == EBSMovementState::InAir)
	{
		UpdateInAirRotation(DeltaTime);
	}

	// Cache values
	PreviousVelocity = GetVelocity();
	PreviousAimYaw = AimingRotation.Yaw;
}

void ABSBaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	MyCharacterMovementComponent = Cast<UBSCharacterMovementComponent>(Super::GetMovementComponent());
}

void ABSBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ABSBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedCurrentAcceleration, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedControlRotation, COND_SkipOwner);
	
	DOREPLIFETIME(ThisClass, DesiredGait);
	DOREPLIFETIME_CONDITION(ThisClass, DesiredStance, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ThisClass, DesiredRotationMode, COND_SkipOwner);

	DOREPLIFETIME_CONDITION(ThisClass, RotationMode, COND_SkipOwner);
}

void ABSBaseCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	// Use the Character Movement Mode changes to set the Movement States to the right values. This allows you to have
	// a custom set of movement states but still use the functionality of the default character movement component.

	if (GetCharacterMovement()->MovementMode == MOVE_Walking ||
		GetCharacterMovement()->MovementMode == MOVE_NavWalking)
	{
		SetMovementState(EBSMovementState::Grounded);
	}
	else if (GetCharacterMovement()->MovementMode == MOVE_Falling)
	{
		SetMovementState(EBSMovementState::InAir);
	}
}

void ABSBaseCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SetStance(EBSStance::Crouching);
}

void ABSBaseCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SetStance(EBSStance::Standing);
}

#pragma region Input
void ABSBaseCharacter::ForwardMovementAction_Implementation(float Value)
{
	if (MovementState == EBSMovementState::Grounded || MovementState == EBSMovementState::InAir)
	{
		// Default camera relative movement behavior
		const FRotator DirRotator(0.0f, AimingRotation.Yaw, 0.0f);
		AddMovementInput(UKismetMathLibrary::GetForwardVector(DirRotator), Value);
	}
}

void ABSBaseCharacter::RightMovementAction_Implementation(float Value)
{
	if (MovementState == EBSMovementState::Grounded || MovementState == EBSMovementState::InAir)
	{
		// Default camera relative movement behavior
		const FRotator DirRotator(0.0f, AimingRotation.Yaw, 0.0f);
		AddMovementInput(UKismetMathLibrary::GetRightVector(DirRotator), Value);
	}
}

void ABSBaseCharacter::CameraUpAction_Implementation(float Value)
{
	AddControllerPitchInput(LookUpDownRate * Value);
}

void ABSBaseCharacter::CameraRightAction_Implementation(float Value)
{
	AddControllerYawInput(LookLeftRightRate * Value);
}

void ABSBaseCharacter::JumpAction_Implementation(bool bValue)
{
	if (bValue)
	{
		if (Stance == EBSStance::Standing)
		{
			Jump();
		}
		else if (Stance == EBSStance::Crouching)
		{
			UnCrouch();
		}
	}
	else
	{
		StopJumping();
	}
}

void ABSBaseCharacter::RunAction_Implementation(bool bValue)
{
	if (bValue)
	{
		SetDesiredGait(EBSGait::Running);
	}
	else
	{
		SetDesiredGait(EBSGait::Walking);
	}
}

void ABSBaseCharacter::AimAction_Implementation(bool bValue)
{
	if (bValue)
	{
		// AimAction: Hold "AimAction" to enter the aiming mode, release to revert back the desired rotation mode.
		SetRotationMode(EBSRotationMode::Aiming);
	}
	else
	{
		SetRotationMode(DesiredRotationMode);
	}
}

void ABSBaseCharacter::StanceAction_Implementation()
{
	UWorld* World = GetWorld();
	check(World);

	if (MovementState == EBSMovementState::Grounded)
	{
		if (Stance == EBSStance::Standing)
		{
			SetDesiredStance(EBSStance::Crouching);
			Crouch();
		}
		else if (Stance == EBSStance::Crouching)
		{
			SetDesiredStance(EBSStance::Standing);
			UnCrouch();
		}
	}
}

void ABSBaseCharacter::VelocityDirectionAction_Implementation()
{
	// Select Rotation Mode: Switch the desired (default) rotation mode to Velocity or Looking Direction.
	// This will be the mode the character reverts back to when un-aiming
	SetDesiredRotationMode(EBSRotationMode::VelocityDirection);
	SetRotationMode(EBSRotationMode::VelocityDirection);
}

void ABSBaseCharacter::LookingDirectionAction_Implementation()
{
	SetDesiredRotationMode(EBSRotationMode::LookingDirection);
	SetRotationMode(EBSRotationMode::LookingDirection);
}

void ABSBaseCharacter::SetDesiredRotationMode(EBSRotationMode NewRotMode)
{
	DesiredRotationMode = NewRotMode;
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetDesiredRotationMode(NewRotMode);
	}
}

void ABSBaseCharacter::SetDesiredGait(EBSGait NewGait)
{
	DesiredGait = NewGait;
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetDesiredGait(NewGait);
	}
}

void ABSBaseCharacter::SetDesiredStance(EBSStance NewStance)
{
	DesiredStance = NewStance;
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetDesiredStance(NewStance);
	}
}

void ABSBaseCharacter::Server_SetDesiredStance_Implementation(EBSStance NewStance)
{
	SetDesiredStance(NewStance);
}

void ABSBaseCharacter::Server_SetDesiredGait_Implementation(EBSGait NewGait)
{
	SetDesiredGait(NewGait);
}

void ABSBaseCharacter::Server_SetDesiredRotationMode_Implementation(EBSRotationMode NewRotMode)
{
	SetDesiredRotationMode(NewRotMode);
}
#pragma endregion

void ABSBaseCharacter::SetMovementState(EBSMovementState NewState, bool bForce)
{
	if (bForce || MovementState != NewState)
	{
		PrevMovementState = MovementState;
		MovementState = NewState;
		OnMovementStateChanged(PrevMovementState);
	}
}

void ABSBaseCharacter::SetGait(EBSGait NewGait, bool bForce)
{
	if (bForce || Gait != NewGait)
	{
		const EBSGait Prev = Gait;
		Gait = NewGait;
		OnGaitChanged(Prev);
	}
}

void ABSBaseCharacter::SetStance(EBSStance NewStance, bool bForce)
{
	if (bForce || Stance != NewStance)
	{
		const EBSStance Prev = Stance;
		Stance = NewStance;
		OnStanceChanged(Prev);
	}
}

void ABSBaseCharacter::SetRotationMode(EBSRotationMode NewRotationMode, bool bForce)
{
	if (bForce || RotationMode != NewRotationMode)
	{
		const EBSRotationMode Prev = RotationMode;
		RotationMode = NewRotationMode;
		OnRotationModeChanged(Prev);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetRotationMode(NewRotationMode, bForce);
		}
	}
}

void ABSBaseCharacter::Server_SetRotationMode_Implementation(EBSRotationMode NewRotationMode, bool bForce)
{
	SetRotationMode(NewRotationMode, bForce);
}

void ABSBaseCharacter::OnRotationModeChanged(EBSRotationMode PreviousRotationMode)
{
	if (CameraBehavior)
	{
		CameraBehavior->SetRotationMode(RotationMode);
	}

	MyCharacterMovementComponent->SetMovementSettings(GetTargetMovementSettings());
}

FTransform ABSBaseCharacter::GetThirdPersonPivotTarget()
{
	return GetActorTransform();
}

ECollisionChannel ABSBaseCharacter::GetThirdPersonTraceParams(FVector& TraceOrigin, float& TraceRadius)
{
	TraceOrigin = GetActorLocation();
	TraceRadius = 10.0f;
	return ECC_Camera;
}

void ABSBaseCharacter::OnRep_RotationMode(EBSRotationMode PrevRotMode)
{
	OnRotationModeChanged(PrevRotMode);
}

float ABSBaseCharacter::GetAnimCurveValue(FName CurveName) const
{
	if (GetMesh()->GetAnimInstance())
	{
		return GetMesh()->GetAnimInstance()->GetCurveValue(CurveName);
	}

	return 0.0f;
}

void ABSBaseCharacter::SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed,
                                               float DeltaTime)
{
	// Interpolate the Target Rotation for extra smooth rotation behavior
	TargetRotation =
		FMath::RInterpConstantTo(TargetRotation, Target, DeltaTime, TargetInterpSpeed);
	SetActorRotation(
		FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, ActorInterpSpeed));
}

void ABSBaseCharacter::SetMovementModel()
{
	const FString ContextString = GetFullName();
	FBSMovementStateSettings* OutRow =
		MovementModel.DataTable->FindRow<FBSMovementStateSettings>(MovementModel.RowName, ContextString);
	check(OutRow);
	MovementData = *OutRow;
}

void ABSBaseCharacter::ForceUpdateCharacterState()
{
	SetGait(DesiredGait, true);
	SetStance(DesiredStance, true);
	SetRotationMode(DesiredRotationMode, true);
	SetMovementState(MovementState, true);
}

float ABSBaseCharacter::CalculateGroundedRotationRate() const
{
	// Calculate the rotation rate by using the current Rotation Rate Curve in the Movement Settings.
	// Using the curve in conjunction with the mapped speed gives you a high level of control over the rotation
	// rates for each speed. Increase the speed if the camera is rotating quickly for more responsive rotation.

	const float MappedSpeedVal = MyCharacterMovementComponent->GetMappedSpeed();
	const float CurveVal =
		MyCharacterMovementComponent->CurrentMovementSettings.RotationRateCurve->GetFloatValue(MappedSpeedVal);
	const float ClampedAimYawRate = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 300.0f}, {1.0f, 3.0f}, AimYawRate);
	return CurveVal * ClampedAimYawRate;
}

void ABSBaseCharacter::LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime)
{
	// Prevent the character from rotating past a certain angle.
	FRotator Delta = AimingRotation - GetActorRotation();
	Delta.Normalize();
	const float RangeVal = Delta.Yaw;

	if (RangeVal < AimYawMin || RangeVal > AimYawMax)
	{
		const float ControlRotYaw = AimingRotation.Yaw;
		const float TargetYaw = ControlRotYaw + (RangeVal > 0.0f ? AimYawMin : AimYawMax);
		SmoothCharacterRotation({0.0f, TargetYaw, 0.0f}, 0.0f, InterpSpeed, DeltaTime);
	}
}

void ABSBaseCharacter::OnGaitChanged(EBSGait PreviousGait)
{
	if (CameraBehavior)
	{
		CameraBehavior->Gait = Gait;
	}
}

void ABSBaseCharacter::OnStanceChanged(EBSStance PreviousStance)
{
	if (CameraBehavior)
	{
		CameraBehavior->Stance = Stance;
	}

	MyCharacterMovementComponent->SetMovementSettings(GetTargetMovementSettings());
}

void ABSBaseCharacter::OnMovementStateChanged(EBSMovementState PreviousState)
{
	if (MovementState == EBSMovementState::InAir)
	{
		// If the character enters the air, set the In Air Rotation and uncrouch if crouched.
		InAirRotation = GetActorRotation();
		if (Stance == EBSStance::Crouching)
		{
			UnCrouch();
		}
	}
}

void ABSBaseCharacter::SetEssentialValues(float DeltaTime)
{
	if (GetLocalRole() != ROLE_SimulatedProxy)
	{
		ReplicatedCurrentAcceleration = GetCharacterMovement()->GetCurrentAcceleration();
		ReplicatedControlRotation = GetControlRotation();
		EasedMaxAcceleration = GetCharacterMovement()->GetMaxAcceleration();
	}
	else
	{
		EasedMaxAcceleration = GetCharacterMovement()->GetMaxAcceleration() != 0
			                       ? GetCharacterMovement()->GetMaxAcceleration()
			                       : EasedMaxAcceleration / 2;
	}

	// Interp AimingRotation to current control rotation for smooth character rotation movement. Decrease InterpSpeed
	// for slower but smoother movement.
	AimingRotation = FMath::RInterpTo(AimingRotation, ReplicatedControlRotation, DeltaTime, 30);

	// These values represent how the capsule is moving as well as how it wants to move, and therefore are essential
	// for any data driven animation system. They are also used throughout the system for various functions,
	// so I found it is easiest to manage them all in one place.

	const FVector CurrentVel = GetVelocity();

	// Set the amount of Acceleration.
	const FVector NewAcceleration = (CurrentVel - PreviousVelocity) / DeltaTime;
	Acceleration = NewAcceleration.IsNearlyZero() || IsLocallyControlled() ? NewAcceleration : Acceleration / 2;

	// Determine if the character is moving by getting it's speed. The Speed equals the length of the horizontal (x y)
	// velocity, so it does not take vertical movement into account. If the character is moving, update the last
	// velocity rotation. This value is saved because it might be useful to know the last orientation of movement
	// even after the character has stopped.
	Speed = CurrentVel.Size2D();
	bIsMoving = Speed > 1.0f;
	if (bIsMoving)
	{
		LastVelocityRotation = CurrentVel.ToOrientationRotator();
	}

	// Determine if the character has movement input by getting its movement input amount.
	// The Movement Input Amount is equal to the current acceleration divided by the max acceleration so that
	// it has a range of 0-1, 1 being the maximum possible amount of input, and 0 being none.
	// If the character has movement input, update the Last Movement Input Rotation.
	MovementInputAmount = ReplicatedCurrentAcceleration.Size() / EasedMaxAcceleration;
	bHasMovementInput = MovementInputAmount > 0.0f;
	if (bHasMovementInput)
	{
		LastMovementInputRotation = ReplicatedCurrentAcceleration.ToOrientationRotator();
	}

	// Set the Aim Yaw rate by comparing the current and previous Aim Yaw value, divided by Delta Seconds.
	// This represents the speed the camera is rotating left to right.
	AimYawRate = FMath::Abs((AimingRotation.Yaw - PreviousAimYaw) / DeltaTime);
}

void ABSBaseCharacter::UpdateCharacterMovement()
{
	// Set the Allowed Gait
	const EBSGait AllowedGait = GetAllowedGait();

	// Determine the Actual Gait. If it is different from the current Gait, Set the new Gait Event.
	const EBSGait ActualGait = GetActualGait(AllowedGait);

	if (ActualGait != Gait)
	{
		SetGait(ActualGait);
	}

	// Update the Character Max Walk Speed to the configured speeds based on the currently Allowed Gait.
	MyCharacterMovementComponent->SetAllowedGait(AllowedGait);
}

void ABSBaseCharacter::UpdateGroundedRotation(float DeltaTime)
{
	const bool bCanUpdateMovingRot = ((bIsMoving && bHasMovementInput) || Speed > 150.0f) && !HasAnyRootMotion();
	if (bCanUpdateMovingRot)
	{
		const float GroundedRotationRate = CalculateGroundedRotationRate();
		if (RotationMode == EBSRotationMode::VelocityDirection)
		{
			// Velocity Direction Rotation
			SmoothCharacterRotation({0.0f, LastVelocityRotation.Yaw, 0.0f}, 800.0f, GroundedRotationRate,
									DeltaTime);
		}
		else if (RotationMode == EBSRotationMode::LookingDirection)
		{
			// Looking Direction Rotation
			const float YawOffsetCurveVal = GetAnimCurveValue(NAME_YawOffset);
			const float YawValue = AimingRotation.Yaw + YawOffsetCurveVal;
		
			SmoothCharacterRotation({0.0f, YawValue, 0.0f}, 500.0f, GroundedRotationRate, DeltaTime);
		}
		else if (RotationMode == EBSRotationMode::Aiming)
		{
			const float ControlYaw = AimingRotation.Yaw;
			SmoothCharacterRotation({0.0f, ControlYaw, 0.0f}, 1000.0f, 20.0f, DeltaTime);
		}
	}
	else
	{
		// Not Moving
		if (RotationMode == EBSRotationMode::Aiming)
		{
			LimitRotation(-100.0f, 100.0f, 20.0f, DeltaTime);
		}

		// Apply the RotationAmount curve from Turn In Place Animations.
		// The Rotation Amount curve defines how much rotation should be applied each frame,
		// and is calculated for animations that are animated at 30fps.

		const float RotAmountCurve = GetAnimCurveValue(NAME_RotationAmount);

		if (FMath::Abs(RotAmountCurve) > 0.001f)
		{
			if (GetLocalRole() == ROLE_AutonomousProxy)
			{
				TargetRotation.Yaw = UKismetMathLibrary::NormalizeAxis(
					TargetRotation.Yaw + (RotAmountCurve * (DeltaTime / (1.0f / 30.0f))));
				SetActorRotation(TargetRotation);
			}
			else
			{
				AddActorWorldRotation({0, RotAmountCurve * (DeltaTime / (1.0f / 30.0f)), 0});
			}
			TargetRotation = GetActorRotation();
		}
	}
}

void ABSBaseCharacter::UpdateInAirRotation(float DeltaTime)
{
	if (RotationMode == EBSRotationMode::VelocityDirection || RotationMode == EBSRotationMode::LookingDirection)
	{
		// Velocity / Looking Direction Rotation
		SmoothCharacterRotation({0.0f, InAirRotation.Yaw, 0.0f}, 0.0f, 5.0f, DeltaTime);
	}
	else if (RotationMode == EBSRotationMode::Aiming)
	{
		// Aiming Rotation
		SmoothCharacterRotation({0.0f, AimingRotation.Yaw, 0.0f}, 0.0f, 15.0f, DeltaTime);
		InAirRotation = GetActorRotation();
	}
}

FBSMovementSettings ABSBaseCharacter::GetTargetMovementSettings() const
{
	if (RotationMode == EBSRotationMode::VelocityDirection)
	{
		if (Stance == EBSStance::Standing)
		{
			return MovementData.VelocityDirection.Standing;
		}
		if (Stance == EBSStance::Crouching)
		{
			return MovementData.VelocityDirection.Crouching;
		}
	}
	else if (RotationMode == EBSRotationMode::LookingDirection)
	{
		if (Stance == EBSStance::Standing)
		{
			return MovementData.LookingDirection.Standing;
		}
		if (Stance == EBSStance::Crouching)
		{
			return MovementData.LookingDirection.Crouching;
		}
	}
	else if (RotationMode == EBSRotationMode::Aiming)
	{
		if (Stance == EBSStance::Standing)
		{
			return MovementData.Aiming.Standing;
		}
		if (Stance == EBSStance::Crouching)
		{
			return MovementData.Aiming.Crouching;
		}
	}

	// Default to velocity dir standing
	return MovementData.VelocityDirection.Standing;
}

EBSGait ABSBaseCharacter::GetAllowedGait() const
{
	// Calculate the Allowed Gait. This represents the maximum Gait the character is currently allowed to be in,
	// and can be determined by the desired gait, the rotation mode, the stance, etc. For example,
	// if you wanted to force the character into a walking state while indoors, this could be done here.
	
	if (RotationMode == EBSRotationMode::Aiming)
	{
		return EBSGait::Walking;
	}
	
	if (DesiredGait == EBSGait::Running)
	{
		return CanRun() ? EBSGait::Running : EBSGait::Walking;
	}
	
	return DesiredGait;
}

EBSGait ABSBaseCharacter::GetActualGait(EBSGait AllowedGait) const
{
	// Get the Actual Gait. This is calculated by the actual movement of the character,  and so it can be different
	// from the desired gait or allowed gait. For instance, if the Allowed Gait becomes walking,
	// the Actual gait will still be running until the character decelerates to the walking speed.

	const float LocWalkSpeed = MyCharacterMovementComponent->CurrentMovementSettings.WalkSpeed;
	// const float LocRunSpeed = MyCharacterMovementComponent->CurrentMovementSettings.RunSpeed;

	if (Speed >= LocWalkSpeed + 10.0f && AllowedGait == EBSGait::Running)
	{
		return EBSGait::Running;
	}

	return EBSGait::Walking;
}

bool ABSBaseCharacter::CanRun() const
{
	// Determine if the character is currently able to run based on the Rotation mode and current acceleration
	// (input) rotation. If the character is in the Looking Rotation mode, only allow running if there is full
	// movement input and it is faced forward relative to the camera + or - 50 degrees.

	if (!bHasMovementInput || RotationMode == EBSRotationMode::Aiming)
	{
		return false;
	}

	const bool bValidInputAmount = MovementInputAmount > 0.9f;

	if (RotationMode == EBSRotationMode::VelocityDirection)
	{
		return bValidInputAmount;
	}

	if (RotationMode == EBSRotationMode::LookingDirection)
	{
		const FRotator AccRot = ReplicatedCurrentAcceleration.ToOrientationRotator();
		FRotator Delta = AccRot - AimingRotation;
		Delta.Normalize();

		return bValidInputAmount && FMath::Abs(Delta.Yaw) < 50.0f;
	}

	return false;
}
