// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/DVRPlayerCharacter.h"

// Engine Includes
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "HeadMountedDisplayFunctionLibrary.h"

// Game Includes
#include "Player./DVRMotionController.h"
#include "Player/DVRPlayerController.h"


// Sets default values
ADVRPlayerCharacter::ADVRPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VRCenter = CreateDefaultSubobject<USceneComponent>(TEXT("VRCenter"));
	VRCenter->SetupAttachment(GetRootComponent());

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(VRCenter);

	CameraCollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CameraCollisionComp"));
	CameraCollisionComp->SetupAttachment(VRCenter);
	CameraCollisionComp->SetSphereRadius(45.f);

	DegreesOnTurn = 45.f;
	bTeleportInProgress = false;
}

/*******************************************************************/
/* Configuration */
/*******************************************************************/
// Called when the game starts or when spawned
void ADVRPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnMotionControllers();


	// For Dev using Index (steamVR)
	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);

	// CameraComp collision detection is from swept movement. This is done slower than game frame rate to save on performance
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_CheckCameraCollision, this, &ADVRPlayerCharacter::CheckForCameraCollision, CameraCollisionCheckRate, true);
	if (CameraComp)
	{
		LastCameraCollisionCompLocation = CameraComp->GetComponentLocation();
	}

	// Cache PlayerCameraManager
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		PlayerCameraManager = PlayerController->PlayerCameraManager;
	}
	
}


void ADVRPlayerCharacter::SpawnMotionControllers()
{
	if (MotionControllerClass && VRCenter)
	{
		FTransform LeftMotionControllerSpawnTransform;
		LeftMotionController = Cast<ADVRMotionController>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, MotionControllerClass, LeftMotionControllerSpawnTransform));
		if (LeftMotionController)
		{
			LeftMotionController->AttachToComponent(VRCenter, FAttachmentTransformRules::KeepRelativeTransform);
			LeftMotionController->SetHand(EControllerHand::Left);
			LeftMotionController->SetVRPlayerCharacter(this);
			LeftMotionController->SetOwner(this);

			UGameplayStatics::FinishSpawningActor(LeftMotionController, LeftMotionControllerSpawnTransform);
		}

		FTransform RightMotionControllerSpawnTransform;
		RightMotionController = Cast<ADVRMotionController>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, MotionControllerClass, RightMotionControllerSpawnTransform));
		if (RightMotionController)
		{
			RightMotionController->AttachToComponent(VRCenter, FAttachmentTransformRules::KeepRelativeTransform);
			RightMotionController->SetHand(EControllerHand::Right);
			RightMotionController->SetVRPlayerCharacter(this);
			RightMotionController->SetOwner(this);

			// Right motion controller is used for UI interaction
			EControllerMode ControllerMode = bUIModeActive ? EControllerMode::ECM_UI : EControllerMode::ECM_Game;
			RightMotionController->SetControllerMode(ControllerMode);

			UGameplayStatics::FinishSpawningActor(RightMotionController, RightMotionControllerSpawnTransform);
		}

		if (LeftMotionController && RightMotionController)
		{
			LeftMotionController->PairController(RightMotionController);
			RightMotionController->PairController(LeftMotionController);
		}
	}
}


void ADVRPlayerCharacter::SetUIModeActive(bool Active)
{
	bUIModeActive = Active;
	EControllerMode ControllerMode = Active ? EControllerMode::ECM_UI : EControllerMode::ECM_Game;
	
	if (RightMotionController)
	{
		RightMotionController->SetControllerMode(ControllerMode);
	}
}


/*******************************************************************/
/* Movement */
/*******************************************************************/
// Called every frame
void ADVRPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AlignRootToVRRoot();
}


void ADVRPlayerCharacter::AlignRootToVRRoot()
{
	if (CameraComp && VRCenter && !bInPauseMenu && !bTeleportInProgress)
	{
		//Align Root component to VRRoot, (room scaling)
		FVector NewCameraOffset = CameraComp->GetComponentLocation() - GetActorLocation();
		NewCameraOffset.Z = 0.f;
		AddActorWorldOffset(NewCameraOffset, true);
		VRCenter->AddWorldOffset(-NewCameraOffset);
	}
}


void ADVRPlayerCharacter::CheckForCameraCollision()
{
	if (CameraCollisionComp && CameraComp && !bTeleportInProgress && !bInPauseMenu)
	{
		// do not allow motion controllers to activate camera collision
		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(this);
		if (LeftMotionController)
		{
			IgnoreActors.Add(LeftMotionController);
			if (LeftMotionController->IsGrabbingActor())
			{
				IgnoreActors.Add(LeftMotionController->GetCurrentGrabbedActor());
			}
		}
		if (RightMotionController)
		{
			IgnoreActors.Add(RightMotionController);
			if (RightMotionController->IsGrabbingActor())
			{
				IgnoreActors.Add(RightMotionController->GetCurrentGrabbedActor());
			}
		}

		FHitResult HitResult;
		FVector CurrentCameraLocation = CameraComp->GetComponentLocation();
		bool Hit = UKismetSystemLibrary::SphereTraceSingle(
			GetWorld(),
			LastCameraCollisionCompLocation,
			CurrentCameraLocation,
			CameraCollisionComp->GetScaledSphereRadius(),
			UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
			false,
			IgnoreActors,
			EDrawDebugTrace::None,
			HitResult,
			true
		);

		if (!bCameraCollisionOverlapping && Hit)
		{
			bCameraCollisionOverlapping = true;
			BP_CollisionStartCameraFade();
		}
		else if (bCameraCollisionOverlapping && !Hit)
		{
			bCameraCollisionOverlapping = false;
			BP_CollisionStopCameraFade();
		}

		CameraCollisionComp->SetWorldLocation(CurrentCameraLocation);
		LastCameraCollisionCompLocation = CurrentCameraLocation;
	}
}


FRotator ADVRPlayerCharacter::GetPlayerViewRotation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentRotation();
	}

	return FRotator(0.f);
}


/*******************************************************************/
/* Input */
/*******************************************************************/
// Called to bind functionality to input
void ADVRPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("GrabLeft"), EInputEvent::IE_Pressed, this, &ADVRPlayerCharacter::GrabLeft);
	PlayerInputComponent->BindAction(TEXT("GrabLeft"), EInputEvent::IE_Released, this, &ADVRPlayerCharacter::ReleaseLeft);

	PlayerInputComponent->BindAction(TEXT("GrabRight"), EInputEvent::IE_Pressed, this, &ADVRPlayerCharacter::GrabRight);
	PlayerInputComponent->BindAction(TEXT("GrabRight"), EInputEvent::IE_Released, this, &ADVRPlayerCharacter::ReleaseRight);

	PlayerInputComponent->BindAction(TEXT("TurnLeft"), EInputEvent::IE_Pressed, this, &ADVRPlayerCharacter::TurnLeft);
	PlayerInputComponent->BindAction(TEXT("TurnRight"), EInputEvent::IE_Pressed, this, &ADVRPlayerCharacter::TurnRight);

	PlayerInputComponent->BindAction(TEXT("StartTeleport"), EInputEvent::IE_Pressed, this, &ADVRPlayerCharacter::StartFindTeleportDestination);
	PlayerInputComponent->BindAction(TEXT("StartTeleport"), EInputEvent::IE_Released, this, &ADVRPlayerCharacter::FinishFindTeleportDestination);

	PlayerInputComponent->BindAction(TEXT("LeftTrigger"), EInputEvent::IE_Pressed, this, &ADVRPlayerCharacter::LeftTriggerPull);
	PlayerInputComponent->BindAction(TEXT("RightTrigger"), EInputEvent::IE_Pressed, this, &ADVRPlayerCharacter::RightTriggerPull);

	PlayerInputComponent->BindAction(TEXT("LeftTrigger"), EInputEvent::IE_Released, this, &ADVRPlayerCharacter::LeftTriggerRelease);
	PlayerInputComponent->BindAction(TEXT("RightTrigger"), EInputEvent::IE_Released, this, &ADVRPlayerCharacter::RightTriggerRelease);
}


void ADVRPlayerCharacter::GrabLeft()
{
	if (LeftMotionController)
	{
		LeftMotionController->Grab();
	}
}


void ADVRPlayerCharacter::ReleaseLeft()
{
	if (LeftMotionController)
	{
		LeftMotionController->Release();
	}
}


void ADVRPlayerCharacter::GrabRight()
{
	if (RightMotionController)
	{
		RightMotionController->Grab();
	}
}


void ADVRPlayerCharacter::ReleaseRight()
{
	if (RightMotionController)
	{
		RightMotionController->Release();
	}
}


void ADVRPlayerCharacter::TurnLeft()
{
	if (VRCenter)
	{
		FRotator Rotation = VRCenter->GetComponentRotation();
		Rotation.Yaw -= DegreesOnTurn;
		VRCenter->SetWorldRotation(Rotation);
	}
}


void ADVRPlayerCharacter::TurnRight()
{
	if (VRCenter)
	{
		FRotator Rotation = VRCenter->GetComponentRotation();
		Rotation.Yaw += DegreesOnTurn;
		VRCenter->SetWorldRotation(Rotation);
	}
}


void ADVRPlayerCharacter::LeftTriggerPull()
{
	if (LeftMotionController)
	{
		LeftMotionController->TriggerPulled();
	}
}


void ADVRPlayerCharacter::LeftTriggerRelease()
{
	if (LeftMotionController)
	{
		LeftMotionController->TriggerReleased();
	}
}


void ADVRPlayerCharacter::RightTriggerPull()
{
	if (RightMotionController)
	{
		RightMotionController->TriggerPulled();
	}
}


void ADVRPlayerCharacter::RightTriggerRelease()
{
	if (RightMotionController)
	{
		RightMotionController->TriggerReleased();
	}
}


void ADVRPlayerCharacter::StartFindTeleportDestination()
{
	if (LeftMotionController && !bWantsToTeleport && !bInPauseMenu)
	{
		bWantsToTeleport = true;
		LeftMotionController->StartFindTeleportDestination();
	}
}


void ADVRPlayerCharacter::FinishFindTeleportDestination()
{
	if (LeftMotionController && bWantsToTeleport && !bInPauseMenu)
	{
		bWantsToTeleport = false;
		FVector TeleportLocation;
		bool ValidTeleportLocation = LeftMotionController->GetCurrentTeleportDestinationMarketLocation(DesiredTeleportLocation);
		if (ValidTeleportLocation)
		{
			LeftMotionController->StopFindTeleportDestination();
			BeginTeleport(false); // do not teleport to pause menu location
		}
	}
}


/*******************************************************************/
/* Teleport */
/*******************************************************************/

void ADVRPlayerCharacter::BeginTeleport(bool TeleportToPauseMenu)
{
	bTeleportInProgress = true;
	float TeleportTimeDelay = 0.f;

	// Teleport to pause menu
	if (!bInPauseMenu && TeleportToPauseMenu)
	{
		ReleaseLeft();
		ReleaseRight();
		TeleportTimeDelay = TeleportToPauseTime;
		SetupTeleportToPauseMenuLocation();
	}
	// teleport from pause menu
	else if (bInPauseMenu && !TeleportToPauseMenu)
	{
		TeleportTimeDelay = TeleportToPauseTime;
		SetupTeleportFromPauseMenuLocation();
	}
	// In game teleport
	else
	{
		TeleportTimeDelay = TeleportTime;
		SetupTeleport();
	}

	OnPlayerBeginTeleport.Broadcast();

	FTimerHandle TimerHandle_TeleportCameraFade;
	GetWorldTimerManager().SetTimer(TimerHandle_TeleportCameraFade, this, &ADVRPlayerCharacter::FinishTeleport, TeleportTimeDelay, false);

}


void ADVRPlayerCharacter::SetupTeleportToPauseMenuLocation()
{
	bInPauseMenu = true;
	StartTeleportCameraFade(0.f, 1.f, 0.f);
	if (RightMotionController)
	{
		RightMotionController->SetControllerMode(EControllerMode::ECM_UI);
	}

	FTimerHandle TimerHandle_Teleport;
	GetWorldTimerManager().SetTimer(TimerHandle_Teleport, this, &ADVRPlayerCharacter::FinishTeleport, 0.2f, false);
}


	
void ADVRPlayerCharacter::SetupTeleportFromPauseMenuLocation()
{
	bInPauseMenu = false;
	StartTeleportCameraFade(0.f, 1.f, 0.f);
	if (RightMotionController)
	{
		RightMotionController->SetControllerMode(EControllerMode::ECM_Game);
	}
}


void ADVRPlayerCharacter::SetupTeleport()
{
	StartTeleportCameraFade(0.f, 1.f, TeleportTime / 2);
}


void ADVRPlayerCharacter::FinishTeleport()
{	
	if (VRCenter)
	{
		// Compute teleport location based on current head mounted display location
		FVector HMDLocation;
		FRotator HMDRotation;
		UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(HMDRotation, HMDLocation);
		HMDLocation.Z = 0.f;
		DesiredTeleportLocation -= HMDLocation;

		VRCenter->SetWorldLocationAndRotation(DesiredTeleportLocation, VRCenter->GetComponentRotation(), false, nullptr, ETeleportType::TeleportPhysics);

		StartTeleportCameraFade(1.f, 0.f, TeleportTime / 2.f);

		OnPlayerFinishTeleport.Broadcast();

		ADVRPlayerController* VRPlayerController = GetController<ADVRPlayerController>();
		if (VRPlayerController)
		{
			VRPlayerController->PlayerCharacterInPauseMenu(bInPauseMenu);
		}
	}

	bTeleportInProgress = false;
}


void ADVRPlayerCharacter::StartTeleportCameraFade(float FromAlpha, float ToAlpha, float Time)
{
	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(FromAlpha, ToAlpha, Time, TeleportCameraFadeColor, false, true);
	}
}


