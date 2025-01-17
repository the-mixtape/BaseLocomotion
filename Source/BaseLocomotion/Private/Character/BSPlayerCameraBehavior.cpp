// Mixtape's Base Locomotion All Rights Reserved.


#include "Character/BSPlayerCameraBehavior.h"


void UBSPlayerCameraBehavior::SetRotationMode(EBSRotationMode RotationMode)
{
	bVelocityDirection = RotationMode == EBSRotationMode::VelocityDirection;
	bLookingDirection = RotationMode == EBSRotationMode::LookingDirection;
	bAiming = RotationMode == EBSRotationMode::Aiming;
}
