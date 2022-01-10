// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DEscapeSuccessVolume.generated.h"


/** Forward declarations */
class UBoxComponent;
class UWidgetComponent;


/**
 * Base class for volume to signify player successfully escaping
 */
UCLASS()
class DUNGEONESCAPEVR_API ADEscapeSuccessVolume : public AActor
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
	UBoxComponent* BoxComp;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UWidgetComponent* SuccessWidgetComp;



/**
 * Methods
 */

public:

	// Sets default values for this actor's properties
	ADEscapeSuccessVolume();

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	/**  Bound callbacks for BoxComp OnBegin and OnEnd overlap events. Will inform GameMode has entered or exited this volume */
	UFUNCTION()
	void OnBoxCompBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnBoxCompEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Show widget for player to decide to quit or travel to main menu */
	void ShowSuccessWidget();

	/** Inform GameMode player has entered the success escape volume, ie BoxComp */
	void InformGameModeEscapeSuccess();

	/** Inform GameMode player has left the success escape volume, ie BoxComp, after successfully escaping */
	void InformGameModePlayerLeftEscapeArea();

};
