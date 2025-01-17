// Mixtape's Base Locomotion All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Library/BSCharacterEnumLibrary.h"
#include "Library/BSCharacterStructLibrary.h"
#include "BSBaseCharacter.generated.h"


class UBSCharacterMovementComponent;
class UBSPlayerCameraBehavior;


UCLASS()
class BASELOCOMOTION_API ABSBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABSBaseCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region CharacterStates
public:
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Character States")
	void SetMovementState(EBSMovementState NewState, bool bForce = false);
	
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Character States")
	void SetGait(EBSGait NewGait, bool bForce = false);
	
	UFUNCTION(BlueprintGetter, Category = "BaseLocomotion|Character States")
	EBSGait GetGait() const { return Gait; }
	
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Character States")
	void SetStance(EBSStance NewStance, bool bForce = false);

	UFUNCTION(BlueprintGetter, Category = "BaseLocomotion|Character States")
	EBSStance GetStance() const { return Stance; }

	UFUNCTION(BlueprintGetter, Category = "BaseLocomotion|Character States")
	EBSRotationMode GetRotationMode() const { return RotationMode; }
	
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Character States")
	void SetRotationMode(EBSRotationMode NewRotationMode, bool bForce = false);
	
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "BaseLocomotion|Character States")
	void Server_SetRotationMode(EBSRotationMode NewRotationMode, bool bForce);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|State Values")
	EBSGait Gait = EBSGait::Walking;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseLocomotion|State Values")
	EBSStance Stance = EBSStance::Standing;

	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|State Values")
	EBSMovementState MovementState = EBSMovementState::None;

	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|State Values")
	EBSMovementState PrevMovementState = EBSMovementState::None;
	
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|State Values", ReplicatedUsing = OnRep_RotationMode)
	EBSRotationMode RotationMode = EBSRotationMode::LookingDirection;

protected:
	virtual void OnRotationModeChanged(EBSRotationMode PreviousRotationMode);
#pragma endregion
	
#pragma region Camera
public:
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Camera System")
	void SetCameraBehavior(UBSPlayerCameraBehavior* CamBeh) { CameraBehavior = CamBeh; }

	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Camera System")
	virtual FTransform GetThirdPersonPivotTarget();
	
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Camera System")
	void GetCameraParameters(float& TPFOVOut) const;
	
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Camera System")
	virtual ECollisionChannel GetThirdPersonTraceParams(FVector& TraceOrigin, float& TraceRadius);
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Camera")
	TObjectPtr<UBSPlayerCameraBehavior> CameraBehavior;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BaseLocomotion|Camera System")
	float ThirdPersonFOV = 90.0f;
#pragma endregion

#pragma region Replication
protected:
	UFUNCTION(Category = "BaseLocomotion|Replication")
	void OnRep_RotationMode(EBSRotationMode PrevRotMode);
#pragma endregion

#pragma region Utils 
protected:
	void SetMovementModel();
	void ForceUpdateCharacterState();
#pragma endregion

private:
	virtual void OnGaitChanged(EBSGait PreviousGait);
	virtual void OnStanceChanged(EBSStance PreviousStance);
	virtual void OnMovementStateChanged(EBSMovementState PreviousState);
	
protected:
	/* Custom movement component*/
	UPROPERTY()
	TObjectPtr<UBSCharacterMovementComponent> MyCharacterMovementComponent;

	/** Input */
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "BaseLocomotion|Input")
	EBSRotationMode DesiredRotationMode = EBSRotationMode::LookingDirection;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "BaseLocomotion|Input")
	EBSGait DesiredGait = EBSGait::Running;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "BaseLocomotion|Input")
	EBSStance DesiredStance = EBSStance::Standing;
	
	/** Movement System */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseLocomotion|Movement System")
	FDataTableRowHandle MovementModel;
	
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Movement System")
	FBSMovementStateSettings MovementData;
	
	/** Rotation System */
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Rotation System")
	FRotator TargetRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Rotation System")
	FRotator InAirRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Rotation System")
	float YawOffset = 0.0f;

	/** Essential Information */
	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Essential Information")
	FRotator LastVelocityRotation;

	UPROPERTY(BlueprintReadOnly, Category = "BaseLocomotion|Essential Information")
	FRotator LastMovementInputRotation;

public:
	UFUNCTION(BlueprintCallable, Category = "BaseLocomotion|Movement System")
	FBSMovementSettings GetTargetMovementSettings() const;
};
