// Mixtape's Base Locomotion All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BSPlayerController.generated.h"


class ABSBaseCharacter;


UCLASS()
class BASELOCOMOTION_API ABSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void OnPossess(APawn* InPawn) override;

protected:
	void SetupCamera();

public:
	/** Main character reference */
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion")
	TObjectPtr<ABSBaseCharacter> PossessedCharacter = nullptr;
};
