// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DEscapeSuccessVolume.generated.h"


/** Forward declarations */
class UBoxComponent;
class UWidgetComponent;


/**
 * Base class for volume to alert game mode player successfully escaped.
 */
UCLASS()
class DUNGEONESCAPEVR_API ADEscapeSuccessVolume : public AActor
{
	GENERATED_BODY()
	

public:

	// Sets default values for this actor's properties
	ADEscapeSuccessVolume();

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	/*******************************************************************/
	/* Components */
	/*******************************************************************/

	/** Root component, when player overlaps with this volume game mode will be alerted that player has successfully escaped */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* BoxComp;

	/** Widget to display (world space) when player is overlapping with BoxComp */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UWidgetComponent* SuccessWidgetComp;


	/*******************************************************************/
	/* Gameplay */
	/*******************************************************************/

	/**  Bound callbacks for BoxComp OnBegin and OnEnd overlap events. Will inform GameMode player has entered or exited BoxComp */
	UFUNCTION()
	void OnBoxCompBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnBoxCompEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Show widget for player to decide to quit or travel to main menu */
	void ShowSuccessWidget();

	/** Inform GameMode player has entered the success escape volume (BoxComp) BoxComp */
	void InformGameModeEscapeSuccess();

	/** Inform GameMode player has left the success escape volume (BoxComp), after successfully escaping */
	void InformGameModePlayerLeftEscapeArea();

};
