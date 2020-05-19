// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupAndRotateActor.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

// Sets default values
APickupAndRotateActor::APickupAndRotateActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CompMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Component Mesh"));
//	CompMesh->SetSimulatePhysics(true);
	//RootComponent = CompMesh;
	CompMesh->SetupAttachment(RootComponent);
	CompMesh->bEditableWhenInherited = true;
	CompMesh->SetupAttachment(RootComponent);

	//ItemType = CreateDefaultSubobject<UActorComponent>(TEXT("Item Type"));



	//sItemType = "";



	if (sItemType != "")
	{

		if (sItemType == "Weapon")
		{
			//SubItemType = CreateDefaultSubobject<UWeaponComponent>(TEXT("Sub Item Type"));
			//ItemType = CreateDefaultSubobject<UWeaponComponent>(TEXT("Item type"));

			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Test Test")));
		}
	}

	cItemType = NULL;
	cSubItemType = NULL;


	bHolding = false;
	bGravity = true;

	CompPos = FVector(50.0f, 0.0f, 0.0f);

}

// Called when the game starts or when spawned
void APickupAndRotateActor::BeginPlay()
{
	Super::BeginPlay();

	MyCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	PlayerCamera = MyCharacter->FindComponentByClass<UCameraComponent>();

	TArray<USceneComponent*> Components;

	MyCharacter->GetComponents(Components);

	if (Components.Num() > 0)
	{
		for (auto& Comp : Components)
		{
			if (Comp->GetName() == "HoldingComponent")
			{
				HoldingComp = Cast<USceneComponent>(Comp);
			}

			if (Comp->GetName() == "The Arrow Component")
			{
				ArrowComp = Cast<USceneComponent>(Comp);
			}
		}
	}
//	TSubclassOf<UWeaponComponent> WeaponSubClass;
	//WeaponSubClass = UWeaponComponent::StaticClass();
	if (sItemType == "Weapon")
	{
		//SubItemType = Cast<UWeaponComponent>(WeaponSubClass);

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("It works")));
	}




	SetItemTypes();
	
}

void APickupAndRotateActor::SetItemTypes()
{

}

void APickupAndRotateActor::LiftActor()
{
	if (bHolding)
	{

		HeightAdjuster += 10;

		if (HeightAdjuster > 160)
		{
			HeightAdjuster = 160;
		}

		CompMesh->SetRelativeLocation(CompPos + FVector(0.0f, 0.0f, HeightAdjuster));

	}

}

void APickupAndRotateActor::LowerActor()
{
	if (bHolding)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("ITs fucking calling something")));

		HeightAdjuster -= 10;

		if (HeightAdjuster < -50)
		{
			HeightAdjuster = -50;
		}

		CompMesh->SetRelativeLocation(CompPos + FVector(0.0f, 0.0f, HeightAdjuster));
	}
}

void APickupAndRotateActor::RotateActor()
{
	ControlRotation = GetWorld()->GetFirstPlayerController()->GetControlRotation();
	SetActorRotation(FQuat(ControlRotation));
}

void APickupAndRotateActor::Pickup()
{
	bHolding = !bHolding;
	bGravity = !bGravity;

	CompMesh->SetEnableGravity(bGravity ? true : false);
	CompMesh->SetSimulatePhysics(bHolding ? false : true);


	sItemType = CompMesh->GetOwner()->GetName();
	//UStaticMeshComponent* ChildMesh = CompMesh->GetChildMeshComponent(0);

//	ChildMesh->SetSimulatePhysics(bHolding);

	CompMesh->SetCollisionEnabled(bHolding ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::QueryAndPhysics);

	if (HoldingComp && bHolding)
	{
		RootComponent->AttachToComponent(HoldingComp, FAttachmentTransformRules::KeepRelativeTransform);
		CompMesh->SetRelativeLocation(CompPos);
	}

	if (!bHolding)
	{
		CompMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		ForwardVector = PlayerCamera->GetForwardVector();
		HeightAdjuster = 0;
		//CompMesh->AddForce(ForwardVector * 100000 * CompMesh->GetMass());
		sItemType = NULL;
	}

}

void APickupAndRotateActor::ThrowHoldingMesh()
{
	bHolding = !bHolding;
	bGravity = !bGravity;

	CompMesh->SetEnableGravity(bGravity ? true : false);
	CompMesh->SetSimulatePhysics(bHolding ? false : true);

	//UStaticMeshComponent* ChildMesh = CompMesh->GetChildMeshComponent(0);

//	ChildMesh->SetSimulatePhysics(bHolding);

	CompMesh->SetCollisionEnabled(bHolding ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);

	if (HoldingComp && bHolding)
	{
		RootComponent->AttachToComponent(HoldingComp, FAttachmentTransformRules::KeepRelativeTransform);
		CompMesh->SetRelativeLocation(CompPos);
	}

	if (!bHolding)
	{
		CompMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		ForwardVector = PlayerCamera->GetForwardVector();
		HeightAdjuster = 0;
		CompMesh->AddForce(ForwardVector * 100000 * CompMesh->GetMass());
		sItemType = NULL;
	}

}

// Called every frame
void APickupAndRotateActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

