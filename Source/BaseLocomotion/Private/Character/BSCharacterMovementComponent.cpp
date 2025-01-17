// Mixtape's Base Locomotion All Rights Reserved.


#include "Character/BSCharacterMovementComponent.h"


void UBSCharacterMovementComponent::SetMovementSettings(FBSMovementSettings NewMovementSettings)
{
	// Set the current movement settings from the owner
	CurrentMovementSettings = NewMovementSettings;
	bRequestMovementSettingsChange = true;
}
