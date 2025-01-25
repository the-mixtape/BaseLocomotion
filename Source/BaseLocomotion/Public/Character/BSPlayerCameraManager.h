// Mixtape's Base Locomotion All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "BSPlayerCameraManager.generated.h"


class USkeletalMeshComponent;
class ABSBaseCharacter;


UCLASS()
class BASELOCOMOTION_API ABSPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, Category = "BaseLocomotion|Camera", meta=(AllowPrivateAccess))
	TObjectPtr<USkeletalMeshComponent> CameraBehavior = nullptr;
	
public:
	ABSPlayerCameraManager();

	UFUNCTION(BlueprintCallable, Category="BaseLocomotion|Camera")
	void OnPossess(ABSBaseCharacter* NewCharacter);
	
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Camera")
	float GetCameraBehaviorParam(FName CurveName) const;

protected:
	virtual void UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Camera")
	static FVector CalculateAxisIndependentLag(
		FVector CurrentLocation, FVector TargetLocation, FRotator CameraRotation, FVector LagSpeeds, float DeltaTime);
	
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Camera")
	bool CustomCameraBehavior(float DeltaTime, FVector& Location, FRotator& Rotation, float& FOV);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BaseLocomotion|Camera")
	TObjectPtr<ABSBaseCharacter> ControlledCharacter = nullptr;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BaseLocomotion|Camera")
	FVector RootLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BaseLocomotion|Camera")
	FTransform SmoothedPivotTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BaseLocomotion|Camera")
	FVector PivotLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BaseLocomotion|Camera")
	FVector TargetCameraLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BaseLocomotion|Camera")
	FRotator TargetCameraRotation;
	
};
