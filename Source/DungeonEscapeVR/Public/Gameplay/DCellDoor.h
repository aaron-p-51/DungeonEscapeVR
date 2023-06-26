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


/** Declare delegate for CellDoorState change */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCellDoorStateChange, AActor*, CellDoor, ECellDoorState, NewState);


/**
 * Base class for cell doors. Cell doors will prevent player from passing until weight on the cell door triggers exceeds weight limit.
 * The process of opening and closing cell doors is driven via time line in derived blueprint. VFX and SFX are spawned in derived blueprint.
 */
UCLASS()
class DUNGEONESCAPEVR_API ADCellDoor : public AActor
{
	GENERATED_BODY()
	
public:

	/* Sets default values for this actor's properties */
	ADCellDoor();

	/** Delegate for Cell door state change */
	FOnCellDoorStateChange OnCellDoorStateChange;

	/* Called every frame */
	virtual void Tick(float DeltaTime) override;

	/** Return mass of all physics actors placed on all instances of ADCellDoorTrigger stored in CellDoorTriggers */
	float CalculateTotalWeightOnTriggers() const;


protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


private:

	/*******************************************************************/
	/* Components */
	/*******************************************************************/

	/** CellDoorStaticMeshComp will move around RootComp */
	UPROPERTY(VisibleDefaultsOnly, Category = "Components", BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	USceneComponent* RootComp;

	/** Cell door Mesh */
	UPROPERTY(EditDefaultsOnly, Category = "Components", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* CellDoorStaticMeshComp;

	/** Blocking collisions when cell door is in ECDS_Closed state */
	UPROPERTY(EditDefaultsOnly, Category = "Components", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* CellDoorBlockingCollision;


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

	/** Check the current weight on all triggers and current CellDoorState. Calls functions to Open/Close cell door if conditions are met */
	void ProcessDoorOpenCloseState();

	/**
	 * Start process of opening cell door. This function is called internally when the weight on all CellDoorTriggeres exceeds WeightToOpenCell. 
	 * CellDoorBlockingCollision will not remain blocking until cell door is completely open. OnCellDoorStateChange will broadcast event when blocking collision is removed.
	 * 
	 * @note cell door movement is driven from Timeline in derived Blueprint class.
	 * @see BP_OpenCellDoor
	 */
	void OpenCellDoor();

	/**
	 * Start process of closing cell door. This function is called internally when the weight on all CellDoorTriggeres is less than WeightToOpenCell.
	 * CellDoorBlockingCollision will start blocking when this function is called. Therefor blocking collisions may be made before visual state of cell door indicated the cell door is closed
	 *  
	 * @note cell door movement is driven from Timeline in derived Blueprint class.
	 * @see BP_CloseCellDoor
	 */
	void CloseCellDoor();


	/**************************************************************************************************/
	/* Cell Door Open/Close. These functions should be called from derived blueprint via timeline */
	/*************************************************************************************************/

protected:

	/** Blueprint event for opening cell door. As cell door opens OnUpdateCellDoorHeight() and OnFinishCellDoorOpened() will be called in derived blueprint */
	UFUNCTION(BlueprintImplementableEvent, Category = "CellDoorState")
	void BP_OpenCellDoor();

	/** Blueprint event for opening cell door. As cell door closes OnUpdateCellDoorHeight() and OnFinishedCellDoorClosed() will be called in derived blueprint */
	UFUNCTION(BlueprintImplementableEvent, Category = "CellDoorState")
	void BP_CloseCellDoor();

	/**
	 * Update the current height of the cell door. This if for visual effect only
	 * The cell door blocking collision is only set CellDoorState is set to ECDS_Opened or ECDS_Closed.
	 * @see OpenCellDoor() and CloseCellDoor()
	 */
	UFUNCTION(BlueprintCallable, Category = "CellDoorState")
	void OnUpdateCellDoorHeight(float CellDoorHeightOffset);

	/** Must be called by derived blueprint when cell door reaches the fully opened position */
	UFUNCTION(BlueprintCallable, Category = "CellDoorState")
	void OnFinishCellDoorOpened();

	/** Must be called by derived blueprint when cell door reaches the fully closed position */
	UFUNCTION(BlueprintCallable, Category = "CellDoorState")
	void OnFinishedCellDoorClosed();


#if WITH_EDITOR

	/*******************************************************************/
	/* Debug */
	/*******************************************************************/

private:

	/** Debug in game mode to force all cell doors to open */
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bGameModeForceAllGatesOpen;

	/** Force this cell door open regardless of weight applied to triggers */
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugForceGateOpen;

#endif

};
