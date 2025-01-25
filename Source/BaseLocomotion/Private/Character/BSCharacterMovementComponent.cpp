// Mixtape's Base Locomotion All Rights Reserved.


#include "Character/BSCharacterMovementComponent.h"

#include "Curves/CurveVector.h"
#include "GameFramework/Character.h"


void UBSCharacterMovementComponent::FSavedMove_My::Clear()
{
	FSavedMove_Character::Clear();
	
	bSavedRequestMovementSettingsChange = false;
	SavedAllowedGait = EBSGait::Walking;
}

uint8 UBSCharacterMovementComponent::FSavedMove_My::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedRequestMovementSettingsChange)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}

void UBSCharacterMovementComponent::FSavedMove_My::SetMoveFor(ACharacter* Character, float InDeltaTime,
	FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	if (const auto CharacterMovement = Cast<UBSCharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		bSavedRequestMovementSettingsChange = CharacterMovement->bRequestMovementSettingsChange;
		SavedAllowedGait = CharacterMovement->AllowedGait;
	}
}

void UBSCharacterMovementComponent::FSavedMove_My::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	if (const auto CharacterMovement = Cast<UBSCharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		CharacterMovement->AllowedGait = SavedAllowedGait;
	}
}

UBSCharacterMovementComponent::FNetworkPredictionData_Client_My::FNetworkPredictionData_Client_My(
	const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement)
{
}

FSavedMovePtr UBSCharacterMovementComponent::FNetworkPredictionData_Client_My::AllocateNewMove()
{
	return MakeShared<FSavedMove_My>();
}

void UBSCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	bRequestMovementSettingsChange = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

FNetworkPredictionData_Client* UBSCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);

	if (!ClientPredictionData)
	{
		UBSCharacterMovementComponent* MutableThis = const_cast<UBSCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_My(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UBSCharacterMovementComponent::OnMovementUpdated(float DeltaTime, const FVector& OldLocation,
	const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaTime, OldLocation, OldVelocity);
	
	if (!CharacterOwner)
	{
		return;
	}

	// Set Movement Settings
	if (bRequestMovementSettingsChange)
	{
		const float UpdateMaxWalkSpeed = CurrentMovementSettings.GetSpeedForGait(AllowedGait);
		MaxWalkSpeed = UpdateMaxWalkSpeed;
		MaxWalkSpeedCrouched = UpdateMaxWalkSpeed;

		bRequestMovementSettingsChange = false;
	}
}

void UBSCharacterMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
	if (CurrentMovementSettings.MovementCurve)
	{
		// Update the Ground Friction using the Movement Curve.
		// This allows for fine control over movement behavior at each speed.
		GroundFriction = CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).Z;
	}
	Super::PhysWalking(deltaTime, Iterations);
}

float UBSCharacterMovementComponent::GetMaxAcceleration() const
{
	// Update the Acceleration using the Movement Curve.
	// This allows for fine control over movement behavior at each speed.
	if (!IsMovingOnGround() || !CurrentMovementSettings.MovementCurve)
	{
		return Super::GetMaxAcceleration();
	}
	return Super::GetMaxAcceleration();
}

float UBSCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	// Update the Deceleration using the Movement Curve.
	// This allows for fine control over movement behavior at each speed.
	if (!IsMovingOnGround() || !CurrentMovementSettings.MovementCurve)
	{
		return Super::GetMaxBrakingDeceleration();
	}
	return Super::GetMaxBrakingDeceleration();
}

float UBSCharacterMovementComponent::GetMappedSpeed() const
{
	// Map the character's current speed to the configured movement speeds with a range of 0-3,
	// with 0 = stopped, 1 = the Walk Speed, 2 = the Run Speed.
	// This allows us to vary the movement speeds but still use the mapped range in calculations for consistent results

	const float Speed = Velocity.Size2D();
	const float LocWalkSpeed = CurrentMovementSettings.WalkSpeed;
	const float LocRunSpeed = CurrentMovementSettings.RunSpeed;

	if (Speed > LocWalkSpeed)
	{
		return FMath::GetMappedRangeValueClamped<float, float>({LocWalkSpeed, LocRunSpeed}, {1.0f, 2.0f}, Speed);
	}

	return FMath::GetMappedRangeValueClamped<float, float>({0.0f, LocWalkSpeed}, {0.0f, 1.0f}, Speed);
}

void UBSCharacterMovementComponent::SetMovementSettings(FBSMovementSettings NewMovementSettings)
{
	// Set the current movement settings from the owner
	CurrentMovementSettings = NewMovementSettings;
	bRequestMovementSettingsChange = true;
}

void UBSCharacterMovementComponent::SetAllowedGait(EBSGait NewAllowedGait)
{
	if (AllowedGait != NewAllowedGait)
	{
		if (PawnOwner->IsLocallyControlled())
		{
			AllowedGait = NewAllowedGait;
			if (GetCharacterOwner()->GetLocalRole() == ROLE_AutonomousProxy)
			{
				Server_SetAllowedGait(NewAllowedGait);
			}
			bRequestMovementSettingsChange = true;
			return;
		}
		if (!PawnOwner->HasAuthority())
		{
			const float UpdateMaxWalkSpeed = CurrentMovementSettings.GetSpeedForGait(AllowedGait);
			MaxWalkSpeed = UpdateMaxWalkSpeed;
			MaxWalkSpeedCrouched = UpdateMaxWalkSpeed;
		}
	}
}

void UBSCharacterMovementComponent::Server_SetAllowedGait_Implementation(EBSGait NewAllowedGait)
{
	AllowedGait = NewAllowedGait;
}
