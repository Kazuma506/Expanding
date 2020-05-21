// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ExpandingCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PickupAndRotateActor.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"


#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>

//////////////////////////////////////////////////////////////////////////
// AExpandingCharacter

AExpandingCharacter::AExpandingCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	//CameraBoom->AttachToComponent(this->GetMesh(), rules, FName("spine_01"));
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	Arrow = CreateDefaultSubobject<USceneComponent>(TEXT("The Arrow Component"));
	Arrow->SetRelativeLocation(FVector(0.0f, 0, 60.0f));

	HoldingComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HoldingComponent"));
	HoldingComponent->SetupAttachment(GetCapsuleComponent());
	//HoldingComponent->RelativeLocation.X = 110.f;

	CurrentItem = NULL;
	bCanMove = true;
	bInspecting = false;
	bThirdPerson = true;

	EquipedItemTest = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon"));
	//EquipedItemTest->AttachToComponent(this->GetMesh(), rules, FName("hand_r"));
}

void AExpandingCharacter::BeginPlay()
{
	Super::BeginPlay();

	PitchMax = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMax;
	PitchMin = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMin;
	YawMax = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewYawMax;
	YawMin = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewYawMin;
	RollMax = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewRollMax;
	RollMin = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewRollMin;

	CameraBoom->AttachToComponent(this->GetMesh(), rules, FName("spine_01"));
}

void AExpandingCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Start = FollowCamera->GetComponentLocation();
	ForwardVector = FollowCamera->GetForwardVector();
	End = ((ForwardVector * 400.0f) + Start);

	//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);

	if (!bHoldingItem)
	{
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, DefaultComponentQueryParams, DefaultResponseParam))
		{
			if (Hit.GetActor()->GetClass()->IsChildOf(APickupAndRotateActor::StaticClass()))
			{
				CurrentItem = Cast<APickupAndRotateActor>(Hit.GetActor());
			}
		}

		else
		{
			CurrentItem = NULL;
		}
	}

	if (bInspecting)
	{
		if (bHoldingItem)
		{
			float yaw = LastRotation.Yaw;
			float roll = LastRotation.Roll;
			float pitch = LastRotation.Pitch;

			CameraBoom->SetWorldRotation(FRotator(pitch, yaw, roll));
			CurrentItem->RotateActor();
		}
	}
	if (CurrentItem == NULL)
	{
		bHoldingItem = false;
		bInspecting = false;
		OnInspectReleased();
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AExpandingCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AExpandingCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AExpandingCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AExpandingCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AExpandingCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AExpandingCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AExpandingCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AExpandingCharacter::OnResetVR);


	PlayerInputComponent->BindAction("PickUpAndHold", IE_Pressed, this, &AExpandingCharacter::OnAction);

	PlayerInputComponent->BindAction("Inspect", IE_Pressed, this, &AExpandingCharacter::OnInspect);
	PlayerInputComponent->BindAction("Inspect", IE_Released, this, &AExpandingCharacter::OnInspectReleased);

	PlayerInputComponent->BindAction("LiftUp", IE_Pressed, this, &AExpandingCharacter::RaiseActor);
	PlayerInputComponent->BindAction("LiftDown", IE_Pressed, this, &AExpandingCharacter::LowerActor);

	PlayerInputComponent->BindAction("SwitchView", IE_Pressed, this, &AExpandingCharacter::FirstThirdPerson);

	PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &AExpandingCharacter::Throw);
}

void AExpandingCharacter::FirstThirdPerson()
{


	if (bThirdPerson)
	{
		//GetWorld()->GetFirstPlayerController()->SetViewTargetWithBlend(FirstPersonCamera, 0.75f);
		CameraBoom->TargetArmLength = -10;
		CameraBoom->AttachToComponent(this->GetMesh(), rules, FName("head"));
		bThirdPerson = false;
	}
	else
	{
		//	GetWorld()->GetFirstPlayerController()->SetViewTargetWithBlend(FollowCamera, 0.75f);
		bThirdPerson = true;
		CameraBoom->AttachToComponent(this->GetMesh(), rules, FName("spine_01"));
		CameraBoom->TargetArmLength = 300;
	}
}

void AExpandingCharacter::OnAction()
{
	if (CurrentItem && !bInspecting)
	{
		ToggleItemPickup();
	}
}

void AExpandingCharacter::OnInspect()
{
	if (bHoldingItem)
	{
		FVector CameraBoomPos = CameraBoom->GetComponentLocation();
		LastRotation = GetControlRotation();

		CameraBoom->bUsePawnControlRotation = false;

		ToggleMovement();
	}
	else
	{
		bInspecting = true;
	}
}

void AExpandingCharacter::OnInspectReleased()
{
	if (bInspecting && bHoldingItem)
	{
		GetController()->SetControlRotation(LastRotation);
		CameraBoom->bUsePawnControlRotation = true;
		ToggleMovement();
	}
	else
	{
		bInspecting = false;
	}
}

void AExpandingCharacter::ToggleMovement()
{
	bCanMove = !bCanMove;
	bInspecting = !bInspecting;
}

void AExpandingCharacter::ToggleItemPickup()
{
	if (CurrentItem)
	{
		bHoldingItem = !bHoldingItem;
		CurrentItem->Pickup();

		if (!bHoldingItem)
		{
			CurrentItem = NULL;
		}
	}
}

void AExpandingCharacter::Throw()
{
	if (CurrentItem)
	{
		bHoldingItem = !bHoldingItem;
		CurrentItem->ThrowHoldingMesh();

		if (!bHoldingItem)
		{
			CurrentItem = NULL;
		}
	}
}

void AExpandingCharacter::ResetPickUp()
{
	CurrentItem = NULL;
	bHoldingItem = false;
	bInspecting = false;
}

bool AExpandingCharacter::GetbHoldingItem()
{
	return bHoldingItem;
}

void AExpandingCharacter::RaiseActor()
{

	CurrentItem->LiftActor();

}

void AExpandingCharacter::LowerActor()
{

	CurrentItem->LowerActor();
}

void AExpandingCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AExpandingCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AExpandingCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AExpandingCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AExpandingCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AExpandingCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AExpandingCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
