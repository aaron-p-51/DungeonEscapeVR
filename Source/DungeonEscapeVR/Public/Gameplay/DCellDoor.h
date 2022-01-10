// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DCellDoor.generated.h"


/** Forward declarations */
class ADCellDoorTrigger;
class UStaticMeshComponent;
class UBoxComponent;
class UParticleSystem;


/**
 * States for Cell Door
 */
UENUM(BlueprintType)
enum class ECellDoorState : uint8
{
	ECDS_Closed		UMETA(DisplayName = "Closed"),
	ECDS_Opening	UMETA(DisplayName = "Opening"),
	ECDS_Opened		UMETA(DisplayName = "Opened"),
	ECDS_Closing	UMETA(DisplayName = "Closing")
};


/** Decalre delegate for CellDoorState change */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCellDoorStateChange, AActor*, CellDoor, ECellDoorState, NewState);


/**
 * Base class for cell doors
 */
UCLASS()
class DUNGEONESCAPEVR_API ADCellDoor : public AActor
{
	GENERATED_BODY()
	
/**
 * Members
 */
private:

	/*******************************************************************/
	/* Components */
	/*******************************************************************/

	/** CellDoorStaticMeshComp will move around RootComp */
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USceneComponent* RootComp;

	/** Cell door Mesh */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UStaticMeshComponent* CellDoorStaticMeshComp;

	/** Blocking collisions when cell door is in ECDS_Closed state */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UBoxComponent* BoxComp;


	/*******************************************************************/
	/* Config */
	/*******************************************************************/

	/** Triggers to open cell door */
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<ADCellDoorTrigger*> CellDoorTriggers;

	/** Total weight that must be applied to CellDoorTriggers in any combination to open CellDoor  */
	UPROPERTY(EditAnywhere, Category = "Config")
	float WeightToOpenCell;


	/*******************************************************************/
	/* State */
	/*******************************************************************/

	/** Height of cell door set on BeginPlay. Used as cell door closed height */
	UPROPERTY()
	float InitialCellDoorHeight;

	/** Current state of this cell door */
	UPROPERTY(VisibleAnywhere, Category = "State")
	ECellDoorState CellDoorState;


	/*******************************************************************/
	/* Debug */
	/*******************************************************************/

	/** Debug in game mode to force all cell doors to open */
	UPROPERTY()
	bool bGameModeForceAllGatesOpen;

	/** Force this cell door open regardless of weight applied to triggers */
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugForceGateOpen;


public:

	/** Delegate for Cell door state change */
	FOnCellDoorStateChange OnCellDoorStateChange;


/**
 * Methods
 */

public:

	/* Sets default values for this actor's properties */
	ADCellDoor();

	/* Called every frame */
	virtual void Tick(float DeltaTime) override;


protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



	/*******************************************************************/
	/* Cell Door State Process */
	/*******************************************************************/

private:

	/** Check the current weight on all triggers and current CellDoorState. Calls functions to Open/Close cell door if conditions are met */
	void ProcessDoorOpenCloseState();

	/** Return mass of all physics actors placed on all instances of ADCellDoorTrigger stored in CellDoorTriggers */
	float CalculateTotalWeightOnTriggers() const;


	/*******************************************************************/
	/* Cell Door Open/Close */
	/*******************************************************************/

	/** Start process of opening cell door. Cell door will not be opened until OnFinishCellDoorOpened is called  */
	void OpenCellDoor();
	/** Start process of closing cell door. Cell door will not be closed until OnFinishedCellDoorClosed is called  */
	void CloseCellDoor();

protected:

	/** Blueprint event for controlling cell door height triggering event based on cell door changing through states. */
	UFUNCTION(BlueprintImplementableEvent)
	void BP_OpenCellDoor();

	/** Blueprint event for controlling cell door height triggering event based on cell door changing through states */
	UFUNCTION(BlueprintImplementableEvent)
	void BP_CloseCellDoor();

	/** Update the current of the cell door. */
	UFUNCTION(BlueprintCallable)
	void OnUpdateCellDoorHeight(float CellDoorHeightOffset);

	/** Must be called by derived BP when cell door reaches the fully opened position */
	UFUNCTION(BlueprintCallable)
	void OnFinishCellDoorOpened();

	/** Must be called by derived BP when cell door reaches the fully closed position */
	UFUNCTION(BlueprintCallable)
	void OnFinishedCellDoorClosed();


};
