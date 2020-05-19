// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ExpandingCharacter.generated.h"

class UInputComponent;
class APickupandRotateActor;

UCLASS(config=Game)
class AExpandingCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AExpandingCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere)
		class APickupAndRotateActor* CurrentItem;

	bool bCanMove;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bHoldingItem;

	bool bInspecting;

	float PitchMax;
	float PitchMin;
	float YawMax;
	float YawMin;
	float RollMax;
	float RollMin;

	FVector HoldingCompent;
	FRotator LastRotation;

	FVector Start;
	FVector ForwardVector;
	FVector End;

	FHitResult Hit;

	FComponentQueryParams DefaultComponentQueryParams;
	FCollisionResponseParams DefaultResponseParam;

	UPROPERTY(EditAnywhere)
		class USceneComponent* HoldingComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		class USceneComponent* EquipedItemTest;

protected:

	virtual void BeginPlay();

	virtual void Tick(float DeltaSeconds) override;

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);


	void OnAction();


	void OnInspect();
	void OnInspectReleased();

	UFUNCTION(BlueprintCallable, Category = "C++ Functions")
		void ToggleItemPickup();

	void ToggleMovement();

	void RaiseActor();
	void LowerActor();

	void FirstThirdPerson();

	UFUNCTION(BlueprintCallable, Category = "C++ Functions")
		void ResetPickUp();

	UFUNCTION(BlueprintCallable, Category = "C++ Functions")
		bool GetbHoldingItem();

	void Throw();

	bool bThirdPerson;



	UPROPERTY(EditAnywhere)
		class USceneComponent* Arrow;

	//UPROPERTY()
		//class UStaticMeshComponent* ArrowMesh;

	FAttachmentTransformRules rules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

