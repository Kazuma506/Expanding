// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/Image.h"
#include "PickupAndRotateActor.generated.h"

class UCameraComponent;
class ACharacter;

UCLASS()
class EXPANDING_API APickupAndRotateActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupAndRotateActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleANywhere, BlueprintReadOnly, Category = "Default")
		UStaticMeshComponent* CompMesh;

	UPROPERTY(EditAnywhere)
		USceneComponent* HoldingComp;

	UPROPERTY(EditAnywhere)
		USceneComponent* ArrowComp;

	UPROPERTY(EditAnywhere)
		UActorComponent* ItemType;

	//UPROPERTY(EditAnywhere)
	//	class UWeaponComponent* SubItemType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FString sItemType;

	//UPROPERTY()
		//class UWeaponComponent* WeaponComponent;

	void ThrowHoldingMesh();

	void SetItemTypes();

	char cItemType;
	char cSubItemType;

	FVector CompPos;

	void LiftActor();
	void LowerActor();

	float HeightAdjuster;

	UPROPERTY()
		class AExpandingCharacter* Player;

	UFUNCTION()
		void RotateActor();

	UFUNCTION()
		void Pickup();

	bool bHolding;
	bool bGravity;

	FRotator ControlRotation;
	ACharacter* MyCharacter;
	UCameraComponent* PlayerCamera;
	FVector ForwardVector;

};
