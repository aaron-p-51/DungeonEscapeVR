// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DVRPlayerCharacter.generated.h"


/** Forward Declarations */
class UCameraComponent;
class ADVRMotionController;
class USphereComponent;


/** Decalre delegate for player teleporting change */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerBeginTeleport);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerFinishTeleport);


UENUM(BlueprintType)
enum class ECameraCollisionState : uint8
{
	ECCS_FadeOut	UMETA(DisplayName = "FadeOut"),
	ECCS_FadeIn		UMETA(DisplayName = "FadeIn"),
	ECCS_NotFading	UMETA(DisplayName = "NotFading")
};


/**
 * Base class for VR player character
 */
UCLASS()
class DUNGEONESCAPEVR_API ADVRPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

/**
 * Members
 */

private:

	/*******************************************************************/
	/* Components */
	/*******************************************************************/

protected:

	/** Room scale center */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	USceneComponent* VRCenter;

private:

	/** Player's view (HMD)*/
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UCameraComponent* CameraComp;

	/** Camera collision detection. When this component starts to overlap with blocking collision CameraComp will fade out blocking player's view */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USphereComponent* CameraCollisionComp;


	/*******************************************************************/
	/* Config */
	/*******************************************************************/

	/** Amount of degrees player will rotate (left or right) when turning using thumb stick */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float DegreesOnTurn;

	/** Rate to check for CameraCollisionComp overlapping with blocking collisions */
	UPROPERTY(EditDefaultsOnly, Category = "Config|PlayerView")
	float CameraCollisionCheckRate;

	/** Fade out color when camera (players HMD) is overlapping with blocking collision. Prevents player from seeing out of play area */
	UPROPERTY(EditDefaultsOnly, Category = "Config|PlayerView")
	FLinearColor CameraCollisionFadeColor;

	/** Fade out color when player is teleporting. Reduce motion sickness */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Teleport")
	FLinearColor TeleportCameraFadeColor;

	/** Time in seconds for camera to fade to TeleportCameraFadeColor. Started when Teleprot starts */
	UPROPERTY(EditDefaultsOnly, Category = "Config|PlayerView")
	float TeleportCamerFadeOutTime;

	/** Time in seconds for camera to fade out TeleportCameraFadeColor. Started when Teleprot finishes */
	UPROPERTY(EditDefaultsOnly, Category = "Config|PlayerView")
	float TeleportCamerFadeInTime;

	/** Total time to teleport. The actual teleport movement will occure over one frame. This time is for the */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Teleport")
	float TeleportTime;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Teleport")
	float TeleportToPauseTime;


	/*******************************************************************/
	/* Motion Controllers */
	/*******************************************************************/

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ADVRMotionController> MotionControllerClass;

	UPROPERTY(VisibleAnywhere, Category = "Player")
	ADVRMotionController* LeftMotionController;

	UPROPERTY(VisibleAnywhere, Category = "Player")
	ADVRMotionController* RightMotionController;


	/*******************************************************************/
	/* State */
	/*******************************************************************/

	/** True as soon as player starts to look for teleport destination. See StartFindTeleportDestination() and FinishFindTeleportDestination() */
	UPROPERTY(VisibleAnywhere, Category = "State|Teleport")
	bool bWantsToTeleport;

	/** True when player is in process of teleporting. This includes camera fade in and out and character teleporting to new destination */
	UPROPERTY(VisibleAnywhere, Category = "State|Teleport")
	bool bTeleportInProgress;

	/**
	 * When player pauses game they are teleported to separate location in the world. This is to avoid player pausing game and pause menu widget
	 * appearing in a location not visible to the player, for example behind a wall.
	 */
	UPROPERTY(VisibleAnywhere, Category = "State|Pause")
	bool bInPauseMenu;

	/** store UI mode active state before MotionControllers spawn. */
	UPROPERTY(VisibleAnywhere, Category = "State|Controllers")
	bool bUIModeActive;

	/** CameraCollisionComp collision detection is run at slower independent rate from frame rate */
	FTimerHandle TimerHandle_CheckCameraCollision;

	/** Last position of CameraCollisionComp. Used to set bCameraCollisionOverlapping   */
	FVector LastCameraCollisionCompLocation;

	/** Is CameraCollisionComp overlapping with another actor */
	UPROPERTY(VisibleAnywhere, Category = "State|PlayerView")
	bool bCameraCollisionOverlapping;

	ECameraCollisionState CameraCollisionState;

	float CollisionCameraFadeAmount;


	/*******************************************************************/
	/* Cached References */
	/*******************************************************************/

	/** Reference to PlayerCameraManager */
	UPROPERTY()
	APlayerCameraManager* PlayerCameraManager;


protected:

	UPROPERTY(BlueprintReadOnly)
	FVector DesiredTeleportLocation;


public:

	/** Delegates for player teleporting change */
	UPROPERTY()
	FOnPlayerBeginTeleport OnPlayerBeginTeleport;
	UPROPERTY()
	FOnPlayerFinishTeleport OnPlayerFinishTeleport;

/**
 * Methods
 */
public:

	// Sets default values for this character's properties
	ADVRPlayerCharacter();


	/*******************************************************************/
	/* Configuration */
	/*******************************************************************/

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	/** Spawn Motion Controllers of type MotionControllerClass. Sets and configures initial state of LeftMotionController and RightMotionController */
	void SpawnMotionControllers();

public:

	/**
	 * Sets bUIModeActive. Used to store initial state to initialize MotionControllers when they are spawned.
	 * Note for this game design only the RightMotionController will ever be set into UIMode. Functionality is already
	 * in place if LeftMotionControler is the desired act in UIMode state.
	 */
	void SetUIModeActive(bool Active);


	/*******************************************************************/
	/* Teleport */
	/*******************************************************************/

	/** Helper function for fade camera in and out while teleporting */
	void StartTeleportCameraFade(float FromAlpha, float ToAlpha, float Time);

public:

	/**
	 * Manually set the desired teleport location. Use with caution. Normal teleport destination should be set from calling StartFindTeleportDestination,
	 * followed by FinishFindTeleportDestination. This will filter out invalid teleport destinations using MotionControllers
	 */
	void SetDesiredTeleportLocation(FVector Location) { DesiredTeleportLocation = Location; }

	/**
	 * Begin the process of teleporting. Teleporting consists of camera fade out, moving player to DesiredTeleportLocation, and
	 * fading the camera back in. Teleporting happens in one of three ways.
	 *	1. Player teleports to pause menu location
	 *	2. Player teleports from pause menu location
	 *	3. Player teleports to DesiredTeleportLocation found by MotionControllers
	 * 
	 * The teleport type is determined from bInPauseMenu and TeleportToPauseMenu param
	 */
	void BeginTeleport(bool TeleportToPauseMenu);

private:

	/** Setup camera fade and teleport timer to teleport to pause menu location. Will set RightMotionController controller mode to ECM_UI */
	void SetupTeleportToPauseMenuLocation();
	/** Setup camera fade and teleport timer to teleport from pause menu location. Will set RightMotionController controller mode to ECM_Game */
	void SetupTeleportFromPauseMenuLocation();
	/** Setup camera fade to teleport */
	void SetupTeleport();

	/** Move player to DesiredTeleportLocation, and fade camera back in. Alert ADVRPlayerController if this teleport was to the pause menu location */
	void FinishTeleport();


	/*******************************************************************/
	/* Movement */
	/*******************************************************************/
	
	/** Keep center of room scale in the same location. CameraComp and CapsuleComponent will move and player moves around play area */
	void AlignRootToVRRoot();

	/**
	 * As player moves around in room scale setup check for collisions in game. If in game collisions are detected, ie walls, then
	 * fade out camera. Prevents motion sickness. This method call rate can be adjusted via CameraCollisionCheckRate. There is no
	 * need to call this method every frame
	 */
	void CheckForCameraCollision();

	void CameraCollisionFade(float DeltaTime);

protected:

	/** Start camera fade to TeleportCameraFadeColor, see derived BP to adjust rate via timeline */
	UFUNCTION(BlueprintImplementableEvent)
	void BP_CollisionStartCameraFade();

	/** Start camera fade, see derived BP to adjust rate via timeline */
	UFUNCTION(BlueprintImplementableEvent)
	void BP_CollisionStopCameraFade();

	UFUNCTION(BlueprintPure)
	FLinearColor GetCameraCollisionFadeColor() const { return CameraCollisionFadeColor; }

public:

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Get the current rotation player is looking. Will get CameraComp's rotation */
	FRotator GetPlayerViewRotation() const;


	/*******************************************************************/
	/* Input */
	/*******************************************************************/

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	/** Grab and release with left motion controller */
	void GrabLeft();
	void ReleaseLeft();

	/** Grab and release with right motion controller */
	void GrabRight();
	void ReleaseRight();

	/** UI Interaction with right motion controller. See SetUIModeActive() */
	void RightTriggerPull();
	void RightTriggerRelease();

	/** UI Interaction with left motion controller See SetUIModeActive()*/
	void LeftTriggerPull();
	void LeftTriggerRelease();

	/** Turn the VRCenter degrees set in DegreesOnTurn. This will turn the room scale in the game world */
	void TurnLeft();
	void TurnRight();

	/** Start looking for valid teleport destination */
	void StartFindTeleportDestination();
	/** If valid teleport destination is found begin teleport to destination. Will set DesiredTeleportLocation */
	void FinishFindTeleportDestination();

};
