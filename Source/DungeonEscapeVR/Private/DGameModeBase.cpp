// Fill out your copyright notice in the Description page of Project Settings.


#include "DGameModeBase.h"


 //Game Includes
#include "Player/DVRPlayerController.h"


void ADGameModeBase::PlayerEscapeSuccess()
{
	bPlayerEscaped = true;
	ADVRPlayerController* VRPlayerController = GetWorld()->GetFirstPlayerController<ADVRPlayerController>();
	if (VRPlayerController)
	{
		VRPlayerController->OnLevelEscapeSuccess();
	}
}

void ADGameModeBase::PlayerLeftEscapeSuccessArea()
{
	ADVRPlayerController* VRPlayerController = GetWorld()->GetFirstPlayerController<ADVRPlayerController>();
	if (VRPlayerController && bPlayerEscaped)
	{
		VRPlayerController->OnLeaveEscapeSuccessArea();
	}
}

