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
 * Base class for cell door triggers
 */
UCLASS()
class DUNGEONESCAPEVR_API ADCellDoorTrigger : public AActor
{
	GENERATED_BODY()
	
/**
 * Members
 */
private:

	/*******************************************************************/
	/* Components */
	/*******************************************************************/

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USceneComponent* RootComp;

	/** Base mesh for trigger, ie what player interprets as the trigger */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	/** Used to determine CellDoorKeys currently placed on trigger */
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


/**
 * Methods
*/

public:

	// Sets default values for this actor's properties
	ADCellDoorTrigger();

	/** Get the total wight of all cell door keys played on trigger, ie weight of all elements of CellDoorKeys  */
	float GetWeightOnTrigger() const;

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


private:

	/** Bound callbacks for BoxComp OnBegin and OnEnd overlap events. Adds and removes ADInteractableActors from CellDoorKeys Array */
	UFUNCTION()
	void OnBoxCompBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnBoxCompEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);






};
