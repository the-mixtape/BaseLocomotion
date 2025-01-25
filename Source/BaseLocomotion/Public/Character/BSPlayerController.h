// Mixtape's Base Locomotion All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BSPlayerController.generated.h"


class ABSBaseCharacter;
class UInputMappingContext;


UCLASS()
class BASELOCOMOTION_API ABSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_Pawn() override;
	virtual void SetupInputComponent() override;
	virtual void BindActions(UInputMappingContext* Context);

protected:
	void SetupCamera() const;
	void SetupInputs() const;
	
	UFUNCTION()
	void ForwardMovementAction(const FInputActionValue& Value);

	UFUNCTION()
	void RightMovementAction(const FInputActionValue& Value);

	UFUNCTION()
	void CameraUpAction(const FInputActionValue& Value);

	UFUNCTION()
	void CameraRightAction(const FInputActionValue& Value);

	UFUNCTION()
	void JumpAction(const FInputActionValue& Value);

	UFUNCTION()
	void RunAction(const FInputActionValue& Value);

	UFUNCTION()
	void AimAction(const FInputActionValue& Value);

	UFUNCTION()
	void StanceAction(const FInputActionValue& Value);

	UFUNCTION()
	void VelocityDirectionAction(const FInputActionValue& Value);

	UFUNCTION()
	void LookingDirectionAction(const FInputActionValue& Value);

public:
	/** Main character reference */
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion")
	TObjectPtr<ABSBaseCharacter> PossessedCharacter = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BaseLocomotion|Input")
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext = nullptr;
};
