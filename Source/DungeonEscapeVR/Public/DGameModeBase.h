// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DGameModeBase.generated.h"


/**
 * Base class for GameMode
 */
UCLASS()
class DUNGEONESCAPEVR_API ADGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
/**
* Members
*/
private:

	/** For all gate in the world to the open state */
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bForceAllCellDoorsOpen;

public:

	bool bPlayerEscaped = false;

/**
 * Methods
 */

public:

	bool GetForceAllCellDoorsOpen() const { return bForceAllCellDoorsOpen; }

	/** Call when player has successfully escaped from the dungeon */
	void PlayerEscapeSuccess();

	/** Call when player has successfully escaped the dungeon but goes back int dungeon */
	void PlayerLeftEscapeSuccessArea();
};
