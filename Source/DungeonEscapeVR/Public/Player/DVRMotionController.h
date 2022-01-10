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
	

/**
* Members
*/

private:

	/*******************************************************************/
	/* Components */
	/*******************************************************************/

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USceneComponent* RootComp;

	/** VR Hand Mesh */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UMotionControllerComponent* MotionControllerComp;

	/** Menu widget interaction */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UWidgetInteractionComponent* WidgetInteractionComp;

	/** Sphere for interacting with interactable actors */
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

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	EControllerMode ControllerMode;

	/** Adjust hand scale. Scaling of world was adjusted, see World to Meters in World Settings. Makeup for scale change */
	UPROPERTY(EditDefaultsOnly, Category = "Config|HandMesh")
	float HandScale;

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

	/** Reference to other hands DVRMotionController */
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

	UPROPERTY()
	TArray<USplineMeshComponent*> TeleportMeshObjectPool;

	/** Current GrabStat of this DVRMotionController */
	UPROPERTY(VisibleAnywhere, Category = "State|Interaction")
	EGrabState GrabState;

	/** Actor currently grabbed, will be attached via PhysicsConstraintComp  */
	UPROPERTY(VisibleAnywhere, Category = "State|Interaction")
	AActor* CurrentGrabedActor;

	/** Actor implementing physics that was overlapping with InteractionSphereComp last frame  */
	UPROPERTY(VisibleAnywhere, Category = "State|Interaction")
	AActor* PreviousOverlappedPhysicsActor;


/**
 * Methods
 */

 public:

	 // Sets default values for this actor's properties
	 ADVRMotionController();

	 // Called every frame
	 virtual void Tick(float DeltaTime) override;


	/*******************************************************************/
	/* Teleport */
	/*******************************************************************/

private:

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
	 * return				Valid teleport destination found
	 */
	UFUNCTION()
	bool FindTeleportDestination(TArray<FVector>& OutPath, FVector& OutLocation);

	/** Set Actors to be ignored for teleport predict projectile path collision detection  */
	void SetIgnoreActorsForTeleportDestination(TArray<AActor*>& IgnoreActors);

	/**
	 * Set SplineMeshComponent locations along path generated from FindTeleportDesination. Points in Path param are added as SplineMeshComponents to TeleportSplinePath
	 * Generated SplineMeshComponents will be added to TeleportMeshObjectPool
	 */
	void SetTeleportSplineMeshComponents(const TArray<FVector>& Path);

	/** Update TeleportSplinePath for all points on Path, enable visibility */
	void UpdateTeleportSpline(const TArray<FVector>& Path);

	/** Clear all points on TeleportSplinePath. Will not delete any SplineMeshComponents from TeleportMeshObjectPool */
	void ClearTeleportPath();

	/** Set visibility of TeleportDestinationMarker */
	void ShowTeleportDestination(bool Show);

public:

	/** Start looking for teleport position. UpdateTeleportDestination will be called every frame until StopFindTeleportDestination is called */
	UFUNCTION()
	void StartFindTeleportDestination();

	/* Stop looking for valid teleport destination position*/
	UFUNCTION()
	void StopFindTeleportDestination();

	/** Get the current teleport destination location. Only use location if this method returns true */
	bool GetCurrentTeleportDestinationMarketLocation(FVector& Location);


	/*******************************************************************/
	/* Interaction */
	/*******************************************************************/

private:
	
	/** Check for overlapping physics actors. Update Haptic feedback and GrabState */
	void UpdateInteractionFeedback();

	/** Get the nearest Actor Implement physics overlapping with InteractionSphereComp */
	AActor* GetNearestOverlappingPhysicsActor() const;

	/** If InteractionSphere is currently overlapping an Actor implementing physics try to attach to PhysicsConstraintComp */
	bool TryAttachOverlappedActorToPhysicsHandle();

	/** Set MeshComp to MotionControllerComp location. Movement is swept to location.  */
	void UpdateMotionControllerTransform();

	/** Update GrabState state for overlapping physics actors */
	void UpdateGrabState(AActor* CurrentOverlappedPhysicsActor);

	/** Play haptic, player feedback, on motion controller */
	void PlayHapticEffect(UHapticFeedbackEffect_Base* HapticEffect, float Intensity = 1.f);

	/**	If CurrentGrabedActor is a ADInteractableActor alert state of GrabState  */
	void AlertGrabbedActorOfGrabState(EGrabState State) const;

public:

	/** Public methods for MotionController Interaction */
	bool IsGrabbingActor() const { return CurrentGrabedActor != nullptr; }
	AActor* GetCurrentGrabbedActor() const { return CurrentGrabedActor; }
	UFUNCTION(BlueprintCallable)
	EGrabState GetGrabState() const { return GrabState; }


	/*******************************************************************/
	/* Configuration */
	/*******************************************************************/

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	/** Set hand, left or right. Hand relative rotation will be set */
	void SetHand(EControllerHand Hand);

	UFUNCTION(BlueprintCallable)
	EControllerHand GetControllerHand() const { return ControllerHand; }

	/** Set Controller Mode for Game or UI mode. Setting to UI mode will enable WidgetInteractionComp */
	UFUNCTION(BlueprintCallable)
	void SetControllerMode(EControllerMode Mode);

	EControllerMode GetControllerMode() const { return ControllerMode; }

	/** Set reference to other hands ADVRMotionController */
	void PairController(ADVRMotionController* Controller) { OtherHandMotionController = Controller; }

	void SetVRPlayerCharacter(ADVRPlayerCharacter* VRPlayerCharacter) { OwnerVRPlayerCharacter = VRPlayerCharacter; }

	/*******************************************************************/
	/* Input */
	/*******************************************************************/

	/** Activate grab state. Attempt to Overlapping interactable actor */
	void Grab();
	/** Activate release state. Release currently grabbed actor */
	void Release();

	/** Trigger mouse click and via WidgetInteractionComp */
	void TriggerPulled();
	/** Trigger mouse click release via WidgetInteractionComp */
	void TriggerReleased();

};
