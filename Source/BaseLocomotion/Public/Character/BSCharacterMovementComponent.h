// Mixtape's Base Locomotion All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Library/BSCharacterStructLibrary.h"
#include "BSCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class BASELOCOMOTION_API UBSCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()


public:
	UFUNCTION(BlueprintCallable, Category = "Movement Settings")
	void SetMovementSettings(FBSMovementSettings NewMovementSettings);

public:
	// Movement Settings Variables
	UPROPERTY()
	uint8 bRequestMovementSettingsChange = 1;
	
	UPROPERTY()
	EBSGait AllowedGait = EBSGait::Walking;
	
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Movement System")
	FBSMovementSettings CurrentMovementSettings;
};
