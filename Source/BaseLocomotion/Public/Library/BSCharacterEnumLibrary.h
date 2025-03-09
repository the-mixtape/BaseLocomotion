// Mixtape's Base Locomotion All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSCharacterEnumLibrary.generated.h"



/**
 * Character movement state. Note: Also edit related struct in ALSStructEnumLibrary if you add new enums
 */
UENUM(BlueprintType, meta = (ScriptName = "BS_MovementState"))
enum class EBSMovementState : uint8
{
	None,
	Grounded,
	InAir
};


/**
 * Character rotation mode. Note: Also edit related struct in ABSStructEnumLibrary if you add new enums
 */
UENUM(BlueprintType, meta = (ScriptName="BS_RotationMode"))
enum class EBSRotationMode : uint8
{
	VelocityDirection,
	LookingDirection,
	Aiming
};


/**
 * Character gait state. Note: Also edit related struct in ABSStructEnumLibrary if you add new enums
 */
UENUM(BlueprintType, meta = (ScriptName = "BS_Gait"))
enum class EBSGait : uint8
{
	Walking,
	Running
};


/**
 * Character stance. Note: Also edit related struct in ABSStructEnumLibrary if you add new enums
 */
UENUM(BlueprintType, meta = (ScriptName = "BS_Stance"))
enum class EBSStance : uint8
{
	Standing,
	Crouching
};