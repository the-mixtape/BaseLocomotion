// Mixtape's Base Locomotion All Rights Reserved.


#include "Character/BSBaseCharacter.h"

#include "Character/BSCharacterMovementComponent.h"
#include "Character/BSPlayerCameraBehavior.h"
#include "Library/BSCharacterStructLibrary.h"
#include "Net/UnrealNetwork.h"


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

	DOREPLIFETIME_CONDITION(ABSBaseCharacter, RotationMode, COND_SkipOwner);
}

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

void ABSBaseCharacter::GetCameraParameters(float& TPFOVOut) const
{
	TPFOVOut = ThirdPersonFOV;
}

ECollisionChannel ABSBaseCharacter::GetThirdPersonTraceParams(FVector& TraceOrigin, float& TraceRadius)
{
	TraceOrigin = GetActorLocation();
	TraceRadius = 10.0f;
	return ECC_Visibility;
}

void ABSBaseCharacter::OnRep_RotationMode(EBSRotationMode PrevRotMode)
{
	OnRotationModeChanged(PrevRotMode);
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
