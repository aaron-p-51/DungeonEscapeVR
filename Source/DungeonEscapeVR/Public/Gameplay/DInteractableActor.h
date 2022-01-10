// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DInteractableActor.generated.h"


/** Forward Declarations */
class UStaticMeshComponent;
class UPhysicsConstraintComponent;
class USphereComponent;


/**
 * Base class for all actors player can interact with 
 */
UCLASS()
class DUNGEONESCAPEVR_API ADInteractableActor : public AActor
{
	GENERATED_BODY()
	

/**
 * Members
 */

private:

	/*******************************************************************/
	/* Components */
	/*******************************************************************/

	UPROPERTY(EditAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

	/** Determine to show outline on MeshComp based on Actors entering or leaving this SphereComponent  */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* InteractionAlertSphereComp;


	/*******************************************************************/
	/* Configuration */
	/*******************************************************************/

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float Weight;


	/*******************************************************************/
	/* State */
	/*******************************************************************/

	/** Current Component inside InteractionAlertSphereComp. Enable proper overlap collisions on InteractionAlertSphereComp to filter  */
	UPROPERTY(VisibleAnywhere, Category = "State|MeshOoutline")
	UPrimitiveComponent* InteractionAlertTrigger;

	/** Current state of MeshComp outline enabled. */
	UPROPERTY(VisibleAnywhere, Category = "State|MeshOoutline")
	bool bOutlineEnabled;

	/** MeshComp outline is turned off when player is teleporting. Keeps track of player teleport status  */
	UPROPERTY(VisibleAnywhere, Category = "State|MeshOoutline")
	bool bPlayerCharacterTeleporting;

	/** Is this actor currently picked up  */
	UPROPERTY(VisibleAnywhere, Category = "State|Interaction")
	bool bIsPickedUp;


/**
 * Methods
 */

public:

	ADInteractableActor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	/** Setup. Player teleport state will determine if MeshComp outlines are shown. See OnPlayerBeginTeleport, and OnPlayerFinishTeleport */
	void BindPlayerPawnTeleportEvents();


	/*******************************************************************/
	/* Interaction */
	/*******************************************************************/

public:

	/** Call when this Actors is picked up. See DVRMotionController. This Actor will be attached to DVRMotionController physics handle */
	virtual void GrabActor();

	/** Call when this Actors is released. See DVRMotionController. This Actor will be released from DVRMotionController physics handle */
	virtual void ReleaseActor();

	bool GetIsPickedUp() const { return bIsPickedUp; }


	/*******************************************************************/
	/* Mesh Outline */
	/*******************************************************************/

private:

	/** Trigger mesh outline */
	UFUNCTION()
	void OnInteractionAlertSphereCompBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnInteractionAlertSphereCompEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Bound function to OnPlayerBeginTeleport delegate. See ADVRPlayerCharacter. Hide MeshComp outline */
	UFUNCTION()
	void OnPlayerBeingTeleport();

	/** Bound function to OnPlayerFinishTeleport delegate. See ADVRPlayerCharacter. Resume MeshComp outline */
	UFUNCTION()
	void OnPlayerFinishTeleport();

	/**
	 *  Check to see if InteractionAlertTrigger has line of sight to this Actor.
	 *  InteractionAlertTrigger will be set from OnInteractionAlertSphereCompBeginOverlap and OnInteractionAlertSphereCompEndOverlap
	 */
	bool UnobstructedViewToInteractionAlertTrigger() const;

	/** Determine of MeshComp outline should be shown */
	void ProcessShowMeshOutline();

	/** Set MeshComp outline visibility. Outline is achieved via setting CustomDepthStencile values.  */
	void SetEnableMeshCompOutline(bool Enable);


};
