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
	

public:

	/** returns player has reached the end of the dungeon */
	bool GetPlayerEscaped() const { return bPlayerEscaped; }

	/** Call when player has successfully escaped from the dungeon */
	void PlayerEscapeSuccess();

	/** Call when player has successfully escaped the dungeon but goes back into dungeon */
	void PlayerLeftEscapeSuccessArea();

	/** For all gate in the world to the open state */
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bForceAllCellDoorsOpen;

	
private:

	/** Player has escaped. Will be set to true when PlayerEscapeSuccess is called. Will never be set back to false until level restarts */
	bool bPlayerEscaped = false;

};
