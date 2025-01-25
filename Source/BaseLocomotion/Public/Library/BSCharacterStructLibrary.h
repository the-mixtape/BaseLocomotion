// Mixtape's Base Locomotion All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSCharacterEnumLibrary.h"

#include "BSCharacterStructLibrary.generated.h"


class UCurveVector;


USTRUCT(BlueprintType)
struct FBSMovementSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float WalkSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float RunSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TObjectPtr<UCurveVector> MovementCurve = nullptr;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TObjectPtr<UCurveFloat> RotationRateCurve = nullptr;

	float GetSpeedForGait(const EBSGait Gait) const
	{
		switch (Gait)
		{
		case EBSGait::Running:
			return RunSpeed;
		case EBSGait::Walking:
			return WalkSpeed;
		default:
			return RunSpeed;
		}
	}
};


USTRUCT(BlueprintType)
struct FBSMovementStanceSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FBSMovementSettings Standing;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FBSMovementSettings Crouching;
};


USTRUCT(BlueprintType)
struct FBSMovementStateSettings : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FBSMovementStanceSettings VelocityDirection;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FBSMovementStanceSettings LookingDirection;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FBSMovementStanceSettings Aiming;
};