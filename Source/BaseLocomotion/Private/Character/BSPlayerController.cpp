// Mixtape's Base Locomotion All Rights Reserved.


#include "Character/BSPlayerController.h"

#include "Character/BSBaseCharacter.h"
#include "Character/BSPlayerCameraManager.h"

void ABSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	PossessedCharacter = Cast<ABSBaseCharacter>(InPawn);
	if(!IsRunningDedicatedServer())
	{
		SetupCamera();
	}
}

void ABSPlayerController::SetupCamera()
{
	// Call "OnPossess" in Player Camera Manager when possessing a pawn
	ABSPlayerCameraManager* CastedMgr = Cast<ABSPlayerCameraManager>(PlayerCameraManager);
	if (PossessedCharacter && CastedMgr)
	{
		CastedMgr->OnPossess(PossessedCharacter);
	}
}
