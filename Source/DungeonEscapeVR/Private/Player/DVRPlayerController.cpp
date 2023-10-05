// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/DVRPlayerController.h"


// Engine Includes
#include "Kismet/GameplayStatics.h"


// Game Includes
#include "Player/DVRPlayerCharacter.h"


/*******************************************************************/
/* Configuration */
/*******************************************************************/
void ADVRPlayerController::BeginPlay()
{
	Super::BeginPlay();

	VRPlayerCharacter = GetPawn<ADVRPlayerCharacter>();

	bLevelHasPauseLocation = GetPauseMenuLocation();

	PauseMenuLayoutActor = FindFirstActorWithTag(PauseMenuLayoutActorTag);
}


bool ADVRPlayerController::GetPauseMenuLocation()
{
	if (AActor* Actor = FindFirstActorWithTag(PlayerPauseLocationActorTag))
	{
		PlayerPauseMenuLocation = Actor->GetActorLocation();
		return true;
	}

	return false;
}


void ADVRPlayerController::SetVRPlayerCharacterUIModeActive(bool Active)
{
	if (VRPlayerCharacter)
	{
		VRPlayerCharacter->SetUIModeActive(Active);
	}
}


/*******************************************************************/
/* Input */
/*******************************************************************/
void ADVRPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction(TEXT("Pause"), EInputEvent::IE_Pressed, this, &ADVRPlayerController::PauseGame);
}


void ADVRPlayerController::PauseGame()
{
	if (bLevelHasPauseLocation && !UGameplayStatics::IsGamePaused(GetWorld()) && VRPlayerCharacter)
	{
		float VRPlayerCharacterYaw = VRPlayerCharacter->GetPlayerViewRotation().Yaw;
		if (PauseMenuLayoutActor)
		{
			FRotator PauseMenuLayoutActorRotation = PauseMenuLayoutActor->GetActorRotation();
			PauseMenuLayoutActorRotation.Yaw = VRPlayerCharacterYaw;
			PauseMenuLayoutActor->SetActorRotation(PauseMenuLayoutActorRotation);
		}

		PlayerLocationWhenPaused = VRPlayerCharacter->GetActorLocation();
		PlayerLocationWhenPaused.Z = 0;
		VRPlayerCharacter->SetDesiredTeleportLocation(PlayerPauseMenuLocation);
		VRPlayerCharacter->BeginTeleport(true);	// teleport to pause menu location
	}
}


void ADVRPlayerController::ReturnToGame()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	if (VRPlayerCharacter)
	{
		VRPlayerCharacter->SetDesiredTeleportLocation(PlayerLocationWhenPaused);
		VRPlayerCharacter->BeginTeleport(false);
	}
}


/*******************************************************************/
/* Gameplay State progression */
/*******************************************************************/
void ADVRPlayerController::OnLevelEscapeSuccess()
{
	if (VRPlayerCharacter)
	{
		VRPlayerCharacter->SetUIModeActive(true);
	}
}


void ADVRPlayerController::OnLeaveEscapeSuccessArea()
{
	if (VRPlayerCharacter)
	{
		VRPlayerCharacter->SetUIModeActive(false);

	}
}


void ADVRPlayerController::PlayerCharacterInPauseMenu(bool Value)
{
	if (UGameplayStatics::IsGamePaused(GetWorld()) != Value)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), Value);
	}
}


/*******************************************************************/
/* Helper functions */
/*******************************************************************/
AActor* ADVRPlayerController::FindFirstActorWithTag(const FName& Tag) const
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag, FoundActors);
	if (FoundActors.Num() > 0)
	{
		return FoundActors[0];
	}

	return nullptr;
}

