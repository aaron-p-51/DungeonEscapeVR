// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DVRMotionController.generated.h"


/** Forward Declarations */
class USkeletalMeshComponent;
class UMotionControllerComponent;
class USphereComponent;
class UStaticMeshComponent;
class ADVRPlayerCharacter;
class ADInteractableActor;
class UPhysicsConstraintComponent;
class UHapticFeedbackEffect_Base;
class USplineComponent;
class USplineMeshComponent;
class UWidgetInteractionComponent;

/** States for MotionController. */
UENUM(BlueprintType)
enum class EGrabState : uint8
{
	EGS_Grab		UMETA(DisplayName = "Grab"),
	EGS_Release		UMETA(DisplayName = "Release"),
	EGS_CanGrab		UMETA(DisplayName = "CanGrab")
};

/** Player input states for MotionController */
UENUM(BlueprintType)
enum class EControllerMode : uint8
{
	ECM_UI			UMETA(DisplayerName = "UI"),
	ECM_Game		UMETA(DisplayValue = "Game")
};


/**
 * Base class for VR motion controllers
 */

UCLASS()
class DUNGEONESCAPEVR_API ADVRMotionController : public AActor
{
	GENERATED_BODY()
	
public:

	// Sets default values for this actor's properties
	ADVRMotionController();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/*******************************************************************/
	/* Teleporting */
	/*******************************************************************/

	/** Start looking for teleport position. UpdateTeleportDestination will be called every frame until StopFindTeleportDestination is called */
	UFUNCTION()
	void StartFindTeleportDestination();

	/* Stop looking for valid teleport destination position*/
	UFUNCTION()
	void StopFindTeleportDestination();

	/** Get the current teleport destination location.
	 *
	 * @param Location will be overwritten to teleport destination
	 * @return true if location was set and is value. Do not teleport to Location param if returns false
	 */
	bool GetCurrentTeleportDestinationMarketLocation(FVector& OutLocation);

	/** Clear all points on TeleportSplinePath. Will not delete any SplineMeshComponents from TeleportMeshObjectPool */
	void ClearTeleportPath();

	/** Set visibility of TeleportDestinationMarker */
	void ShowTeleportDestination(bool bShow);


	/*******************************************************************/
	/* Grabbing */
	/*******************************************************************/

	/** Check if this MotionController is currently grabbing another actor
	 * @returns true if currently grabbing another actor
	 */
	bool IsGrabbingActor() const { return CurrentGrabedActor != nullptr; }

	/**
	 * Get this DVRMotionController's currently grabbed actor
	 * @returns currently grabbed actor, returns nullptr if no actor is currently grabbed
	 */
	AActor* GetCurrentGrabbedActor() const { return CurrentGrabedActor; }

	/**
	 * Get current grab state. Grab state is only dependent on current player input. Does not depend on
	 * if an actor is currently grabbed or not
	 * 
	 * @see GetCurrentGrabbedActor()
	 */
	UFUNCTION(BlueprintPure)
	EGrabState GetGrabState() const { return GrabState; }

	/*******************************************************************/
	/* Setup */
	/*******************************************************************/

	/** Set hand, left or right. Hand relative rotation will be set
	 * @note only EControllerHand::Left and EControllerHand::Right are valid
	 */
	void SetHand(EControllerHand Hand);

	/** Get this controller's hand
	 * @returns this controllers hand. Can only return EControllerHand::Left or EControllerHand::Right
	 */
	UFUNCTION(BlueprintPure)
	EControllerHand GetControllerHand() const { return ControllerHand; }

	/** Set Controller Mode for Game or UI mode.
	 * Setting to UI mode will enable WidgetInteractionComp
	 * Setting to Game will enable teleporting and grabbing
	 */
	UFUNCTION(BlueprintCallable)
	void SetControllerMode(EControllerMode Mode);

	/** Get the current controller mode */
	UFUNCTION(BlueprintPure)
	EControllerMode GetControllerMode() const { return ControllerMode; }

	/** Set to other hands ADVRMotionController */
	void PairController(ADVRMotionController* Controller) { OtherHandMotionController = Controller; }

	/** Set ADVRPlayerCharacter for player using this motion cotroller */
	void SetVRPlayerCharacter(ADVRPlayerCharacter* VRPlayerCharacter) { OwnerVRPlayerCharacter = VRPlayerCharacter; }

	/*******************************************************************/
	/* Input */
	/*******************************************************************/

	/** Activate grab state. Attempt to Overlapping interactable actor */
	void Grab();
	/** Activate release state. Release currently grabbed actor if one is grabbed */
	void Release();

	/** Trigger mouse click and via WidgetInteractionComp */
	void TriggerPulled() const;
	/** Trigger mouse click release via WidgetInteractionComp */
	void TriggerReleased() const;

	/*******************************************************************/
	/* Interaction */
	/*******************************************************************/

	/** Play haptic, player feedback, on motion controller */
	void PlayHapticEffect(UHapticFeedbackEffect_Base* HapticEffect, float Intensity = 1.f) const;

	/**	If CurrentGrabedActor is a ADInteractableActor alert state of GrabState  */
	void AlertGrabbedActorOfGrabState(EGrabState State) const;

	/** Check for overlapping physics actors. Update Haptic feedback and GrabState */
	void UpdateInteractionWithOverlappingActors();

	/** Get the nearest Actor Implement physics overlapping with InteractionSphereComp */
	AActor* GetNearestOverlappingPhysicsActor() const;

	/** If InteractionSphere is currently overlapping an Actor implementing physics try to attach to PhysicsConstraintComp */
	bool TryAttachOverlappedActorToPhysicsHandle();

	/** Set MeshComp to MotionControllerComp location. Movement is swept to location.  */
	void UpdateMotionControllerTransform();


protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


private:

	/*******************************************************************/
	/* Components */
	/*******************************************************************/

	//UPROPERTY(EditDefaultsOnly, Category = "Components")
	//USceneComponent* RootComp;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UMotionControllerComponent* MotionControllerComp;

	/** VR Hand Mesh */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	/** Menu widget interaction */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UWidgetInteractionComponent* WidgetInteractionComp;

	/** Sphere for interacting with interactable actors. */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USphereComponent* InteractionSphereComp;

	/** Interactable actors are attached when grabbed */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UPhysicsConstraintComponent* PhysicsConstraintComp;

	/** Destination marker for best estimate of position after teleporting */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UStaticMeshComponent* TeleportDestinationMarker;

	/** Spline to show teleport projectile path */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USplineComponent* TeleportSplinePath;


	/*******************************************************************/
	/* Configuration */
	/*******************************************************************/

	/** Current Controller Mode */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	EControllerMode ControllerMode;

	/** Adjust hand scale. Scaling of world was adjusted, see World to Meters in World Settings. Makeup for scale change */
	UPROPERTY(EditDefaultsOnly, Category = "Config|HandMesh")
	float HandScale;

	/** Hand offset from UMotionControllerComponent. MeshComp is swept to prevent going through blocing collisions */
	UPROPERTY(EditDefaultsOnly, Category = "Config|HandMesh")
	FVector RelativeHandPositionOffset;

	/** Teleport Projectile path radius to check for collisions, should be adjusted to match with character capsule comp radius and navmesh settings */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Teleport")
	float TeleportProjectileRadius;

	/** Adjust distance of max teleport location from starting point */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Teleport")
	float TeleportProjectileSpeed;

	/* Maximum simulation time for the virtual projectile */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Teleport")
	float TeleportSimulationTime;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Teleport")
	bool bTeleportTraceComplex;

	/** Mesh for teleport projectile path points */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Teleport|Spline")
	UStaticMesh* TeleportArchMesh;

	/** Materials for teleport projectile path points */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Teleport|Spline")
	UMaterialInterface* TeleportArchMaterial;

	/** Feedback when overlapping interactable actor */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Feedback")
	UHapticFeedbackEffect_Base* CanPickupHapticEffect;


	/*******************************************************************/
	/* State */
	/*******************************************************************/

	UPROPERTY(VisibleAnywhere, Category = "State")
	EControllerHand ControllerHand;

	/** Owner player character */
	UPROPERTY(VisibleAnywhere, Category = "State")
	ADVRPlayerCharacter* OwnerVRPlayerCharacter;

	/** Players other hand DVRMotionController */
	UPROPERTY(VisibleAnywhere, Category = "State")
	ADVRMotionController* OtherHandMotionController;

	/** Set when motion controlled is actively looking for valid teleport destinations */
	UPROPERTY(VisibleAnywhere, Category = "State|Teleport")
	bool bLookForTeleportDestination;

	/** Is the last teleport desination position valid based on last teleport projectile path*/
	UPROPERTY(VisibleAnywhere, Category = "State|Teleport")
	bool bHasValidTeleportDestination;

	/** Current teleport location based on teleport projectile path end point */
	UPROPERTY(VisibleAnywhere, Category = "State|Teleport")
	FVector TeleportDestination;

	/** Cache meshes placed along spline showing path to teleport location  */
	UPROPERTY()
	TArray<USplineMeshComponent*> TeleportMeshObjectPool;

	/** Current GrabState of this DVRMotionController */
	UPROPERTY(VisibleAnywhere, Category = "State|Interaction")
	EGrabState GrabState;

	/** Actor currently grabbed, will be attached via PhysicsConstraintComp  */
	UPROPERTY(VisibleAnywhere, Category = "State|Interaction")
	AActor* CurrentGrabedActor;

	/** Actor implementing physics that was overlapping with InteractionSphereComp last frame  */
	UPROPERTY(VisibleAnywhere, Category = "State|Interaction")
	AActor* PreviousOverlappedPhysicsActor;


	/*******************************************************************/
	/* Teleport */
	/*******************************************************************/

	/**
	 * Update the current teleport destination. Teleport destination is based on a predict projectile path. If updated destination is valid
	 * destination will be shown using TeleportDestinationMarker and projectile path will be shown using TeleportSplinePath. This method is called
	 * every frame as long as bLookForTeleportDestination is true. 
	 */
	UFUNCTION()
	void UpdateTeleportDestination();

	/**
	 * Using predict projectile path and map navmesh to find teleport destination
	 * 
	 * @param OutPath		points along predict projectile path
	 * @param OutLocation	Collision location of predict projectile path
	 * 
	 * @return				Valid teleport destination found
	 */
	UFUNCTION()
	bool FindTeleportDestination(TArray<FVector>& OutPath, FVector& OutLocation);

	/** Set Actors to be ignored for teleport predict projectile path collision detection  */
	void SetIgnoreActorsForTeleportDestination(TArray<AActor*>& OutIgnoreActors);

	/**
	 * Set SplineMeshComponent locations along path generated from FindTeleportDesination. Points in Path param are added as SplineMeshComponents to TeleportSplinePath
	 * Generated SplineMeshComponents will be added to TeleportMeshObjectPool
	 */
	void SetTeleportSplineMeshComponents(const TArray<FVector>& Path);

	/** Update TeleportSplinePath for all points on Path, enable visibility */
	void UpdateTeleportSpline(const TArray<FVector>& Path);


	/*******************************************************************/
	/* Interaction */
	/*******************************************************************/
	
	/** Update GrabState state for overlapping physics actors
	 * @param bIsOverlappingActorToGrab is there an actor currently overlapping InteractionSphereComp
	 */
	void UpdateGrabState(bool bIsOverlappingActorToGrab);

};
