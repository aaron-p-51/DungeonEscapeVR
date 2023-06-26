// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DVRPlayerController.generated.h"

/** Forward class declarations */
class ADVRPlayerCharacter;

/**
 * Base class for VR Player Controller
 */
UCLASS()
class DUNGEONESCAPEVR_API ADVRPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	/**
	 * Start Pausing the game. VRPlayerCharacter will be moved to PlayerPauseMenuLocation. Pause will not be complete until
	 * PlayerCharacterInPauseMenu() is called
	 */
	void PauseGame();

	// Called to bind functionality to input
	virtual void SetupInputComponent() override;

	/** 
	 * Start unpausing the game. Will return VRPlayerCharacter to PlayerLocationWhenPaused. Will not be complete until 
	 * PlayerCharacterInPauseMenu() is called. Note: See menu widgets for when this method is called
	 */
	UFUNCTION(BlueprintCallable)
	void ReturnToGame();

	/**
	 * Find the first actor in world that has tag matching PlayerPauseLocationActorTag. Store actors location as PlayerPauseMenuLocation.
	 * @return true if PlayerPauseMenuLocation has been set
	 */
	bool GetPauseMenuLocation();

	/**
	 * Set VRPlayerCharacter to ECM_UI Mode. Will enable interaction with widgets. If Active param in false will return to ECM_Game Mode.
	 * This method should be called from level blueprint to set initial EControllerMode for VRPlayerCharacter and its motion controllers
	 */
	UFUNCTION(BlueprintCallable)
	void SetVRPlayerCharacterUIModeActive(bool Active);


	/*******************************************************************/
	/* Gameplay State progression */
	/*******************************************************************/

	/** VRPlayerCharacter has escaped the game. Set VRPlayerCharacter to ECM_UI Mode */
	void OnLevelEscapeSuccess();

	/** VRPlayerCharacter has successfully escaped but decided to go back into the dungeon. Set VRPlayerCharacter back to ECM_Game Mode*/
	void OnLeaveEscapeSuccessArea();

	/** Pause or unpause the game. If Value is true game will be paused, and unpaused if false */
	void PlayerCharacterInPauseMenu(bool Value);
	

	/*******************************************************************/
	/* Helper functions */
	/*******************************************************************/

	/** Find first actor in world with Tag. This is slow, calls UGameplayStatics::GetAllActorsWithTag */
	AActor* FindFirstActorWithTag(const FName& Tag) const;


protected:

	virtual void BeginPlay() override;


/**
 * Members
 */
private:

	/*******************************************************************/
	/* Configuration */
	/*******************************************************************/

	/** Actor tag that marks where the world pause location is */
	UPROPERTY(EditAnywhere, Category = "Configuration")
	FName PlayerPauseLocationActorTag = FName(TEXT("PauseLocation"));

	/** Actor tag that marks the layout actor for Pause menu. */
	FName PauseMenuLayoutActorTag = FName(TEXT("PauseMenuWidgetLayout"));

	/*******************************************************************/
	/* Pause */
	/*******************************************************************/

	/**
	 * Position in world where player owning player character will be teleported to when game is paused. Set from actor with tag
	 * matching PlayerPauseLocationActorTag. Owning player character will not be able to pause game map if not set.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Pause")
	FVector PlayerPauseMenuLocation;

	/** Layout actor for pause menu. This actor will be rotated so that pause menu faces player when paused */
	UPROPERTY(VisibleAnywhere, Category = "Pause")
	AActor* PauseMenuLayoutActor;

	/** Has PlayerPauseMenuLocation been set */
	UPROPERTY(VisibleAnywhere, Category = "Pause")
	bool bLevelHasPauseLocation = false;

	/** Position in world where owning player character is when game is paused. Returned to this position when game is unpaused */
	UPROPERTY(VisibleAnywhere, Category = "Pause")
	FVector PlayerLocationWhenPaused;


	/*******************************************************************/
	/* Cached References */
	/*******************************************************************/

	/** Current controlled pawn */
	UPROPERTY()
	ADVRPlayerCharacter* VRPlayerCharacter;

};
