// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DCellDoorTrigger.generated.h"


/** Forward declarations */
class UBoxComponent;
class UStaticMeshComponent;
class ADInteractableActor;


/**
 * Base class for cell door triggers. Cell door triggers are using to open CellDoors
 */
UCLASS()
class DUNGEONESCAPEVR_API ADCellDoorTrigger : public AActor
{
	GENERATED_BODY()
	
public:

	// Sets default values for this actor's properties
	ADCellDoorTrigger();

	/**
	 * Get the total wight of all cell door keys placed on trigger, this is the weight of all elements of CellDoorKeys.
	 * @returns Combined mass of all ADInteractableActors UPrimitiveComponent in CellDoorKeys
	 */
	float GetWeightOnTrigger() const;


protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


private:

	/*******************************************************************/
	/* Components */
	/*******************************************************************/

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USceneComponent* RootComp;

	/** Base mesh for trigger, what player interprets as the trigger */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	/**
	 * Used to determine CellDoorKeys currently placed on trigger. Overlapping ADInteractableActors
	 * with tag matching CellDoorKeyTag will be stored in CellDoorKeys
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UBoxComponent* BoxComp;


	/*******************************************************************/
	/* Config */
	/*******************************************************************/

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	FName CellDoorKeyTag;


	/*******************************************************************/
	/* State */
	/*******************************************************************/

	/** Current CellDoorKeys overlapping BoxComp, ie placed on trigger */
	UPROPERTY()
	TArray<ADInteractableActor*> CellDoorKeys;


	/*******************************************************************/
	/* Gameplay */
	/*******************************************************************/

	/** Bound callbacks for BoxComp OnBegin and OnEnd overlap events. Adds and removes ADInteractableActors from CellDoorKeys Array */
	UFUNCTION()
	void OnBoxCompBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnBoxCompEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
