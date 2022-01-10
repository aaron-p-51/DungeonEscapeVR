// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/DVRMotionController.h"

// Engine Includes
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "MotionControllerComponent.h"
#include "NavigationSystem.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"


// Game Includes
#include "Gameplay/DInteractableActor.h"
#include "Player/DVRPlayerCharacter.h"


// Sets default values
ADVRMotionController::ADVRMotionController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MotionControllerComp = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerComp"));
	SetRootComponent(MotionControllerComp);

	WidgetInteractionComp = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteractionComp"));
	WidgetInteractionComp->SetupAttachment(GetRootComponent());
	WidgetInteractionComp->bShowDebug = true;
	WidgetInteractionComp->DebugColor = FLinearColor::Red;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(GetRootComponent());

	InteractionSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphereComp"));
	InteractionSphereComp->SetupAttachment(MeshComp);
	InteractionSphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphereComp->SetHiddenInGame(false);
	
	PhysicsConstraintComp = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraintComp"));
	PhysicsConstraintComp->SetupAttachment(MeshComp);
	PhysicsConstraintComp->SetDisableCollision(true);
	//PhysicsConstraintComp->SetRelativeLocation(FVector(9.132618f, 0.020011f, 1.277876f));
	PhysicsConstraintComp->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 45.f);
	PhysicsConstraintComp->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, 0.f);
	PhysicsConstraintComp->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Limited, 0.f);
	

	TeleportDestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TeleportDestinationMarker"));
	TeleportDestinationMarker->SetupAttachment(GetRootComponent());

	TeleportSplinePath = CreateDefaultSubobject<USplineComponent>(TEXT("TeleportSplinePath"));
	TeleportSplinePath->SetupAttachment(GetRootComponent());
	
	bLookForTeleportDestination = false;
	TeleportProjectileRadius = 10.f;
	TeleportProjectileSpeed = 800.f;
	TeleportSimulationTime = 2.f;
	bTeleportTraceComplex = true;
	ControllerMode = EControllerMode::ECM_UI;
	HandScale = 1.f;
	CurrentGrabedActor = nullptr;
}


// Called when the game starts or when spawned
void ADVRMotionController::BeginPlay()
{
	Super::BeginPlay();
	
	if (TeleportDestinationMarker)
	{
		TeleportDestinationMarker->SetVisibility(false, true);
	}

	SetControllerMode(ControllerMode);
}


// Called every frame
void ADVRMotionController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateMotionControllerTransform();
	UpdateInteractionFeedback();

	if (bLookForTeleportDestination)
	{
		UpdateTeleportDestination();
	}
}


/*******************************************************************/
/* Interaction */
/*******************************************************************/
void ADVRMotionController::UpdateMotionControllerTransform()
{
	if (MeshComp)
	{
		MeshComp->SetRelativeLocation(RelativeHandPositionOffset, true);
	}
}


void ADVRMotionController::UpdateInteractionFeedback()
{
	AActor* CurrentOverlappedPhysicsActor = GetNearestOverlappingPhysicsActor();

	if (CurrentOverlappedPhysicsActor && CurrentOverlappedPhysicsActor != PreviousOverlappedPhysicsActor &&
		(!OtherHandMotionController || (OtherHandMotionController && OtherHandMotionController->GetCurrentGrabbedActor() != CurrentOverlappedPhysicsActor)))
	{
		PlayHapticEffect(CanPickupHapticEffect);
	}

	UpdateGrabState(CurrentOverlappedPhysicsActor);

	PreviousOverlappedPhysicsActor = CurrentOverlappedPhysicsActor;
}


AActor* ADVRMotionController::GetNearestOverlappingPhysicsActor() const
{
	AActor* NearestActor = nullptr;
	if (InteractionSphereComp)
	{
		FVector InteractionSphereLocation = InteractionSphereComp->GetComponentLocation();
		float NearestActorDistance = MAX_FLT;
		TSet<AActor*> OverlappingActors;

		InteractionSphereComp->GetOverlappingActors(OverlappingActors);

		for (const auto& Actor : OverlappingActors)
		{
			if (Actor->GetRootComponent()->IsSimulatingPhysics())
			{
				float DistanceToActor = FVector::DistSquared(Actor->GetActorLocation(), InteractionSphereLocation);
				if (DistanceToActor < NearestActorDistance)
				{
					NearestActor = Actor;
					NearestActorDistance = DistanceToActor;
				}
			}
		}
	}

	return NearestActor;
}


void ADVRMotionController::PlayHapticEffect(UHapticFeedbackEffect_Base* HapticEffect, float Intensity)
{
	if (HapticEffect && OwnerVRPlayerCharacter)
	{
		APlayerController* MyController = Cast<APlayerController>(OwnerVRPlayerCharacter->GetController());
		if (MyController)
		{
			MyController->PlayHapticEffect(HapticEffect, MotionControllerComp->GetTrackingSource());//, Intensity);
		}
	}
}


void ADVRMotionController::UpdateGrabState(AActor* CurrentOverlappedPhysicsActor)
{
	// state EGS_Grab is set from player input, see Grab()
	if (GrabState == EGrabState::EGS_Grab) return;

	if (CurrentOverlappedPhysicsActor)
	{
		GrabState = EGrabState::EGS_CanGrab;
	}
	else
	{
		GrabState = EGrabState::EGS_Release;
	}
}


bool ADVRMotionController::TryAttachOverlappedActorToPhysicsHandle()
{
	if (PhysicsConstraintComp && PreviousOverlappedPhysicsActor)
	{
		UPrimitiveComponent* OverlappingPhysicsActorRoot = Cast<UPrimitiveComponent>(PreviousOverlappedPhysicsActor->GetRootComponent());
		if (OverlappingPhysicsActorRoot)
		{
			PhysicsConstraintComp->SetConstrainedComponents(InteractionSphereComp, NAME_None, OverlappingPhysicsActorRoot, NAME_None);
			CurrentGrabedActor = PreviousOverlappedPhysicsActor;
			return true;
		}
	}

	return false;
}


void ADVRMotionController::AlertGrabbedActorOfGrabState(EGrabState State) const
{
	// If grabbed actor was of type ADInteractableActor alert Actor is was grabbed
	ADInteractableActor* InteractableActor = Cast<ADInteractableActor>(CurrentGrabedActor);
	if (InteractableActor)
	{
		if (State == EGrabState::EGS_Grab)
		{
			InteractableActor->GrabActor();
		}
		else if (State == EGrabState::EGS_Release)
		{
			InteractableActor->ReleaseActor();
		}
	}
}


/*******************************************************************/
/* Teleport */
/*******************************************************************/
void ADVRMotionController::UpdateTeleportDestination()
{
	if (TeleportDestinationMarker)
	{
		TArray<FVector> Path;
		bHasValidTeleportDestination = FindTeleportDestination(Path, TeleportDestination);

		// draw and show teleport destination and path
		if (bHasValidTeleportDestination)
		{
			TeleportDestinationMarker->SetWorldLocationAndRotation(TeleportDestination, FRotator(0.f));
			ShowTeleportDestination(true);
			SetTeleportSplineMeshComponents(Path);
		}
		else // hide teleport destination marker, clear projectile path
		{
			ShowTeleportDestination(false);
			ClearTeleportPath();
		}
	}
}


bool ADVRMotionController::FindTeleportDestination(TArray<FVector>& OutPath, FVector& OutLocation)
{
	if (MotionControllerComp)
	{
		FVector Start = MotionControllerComp->GetComponentLocation();
		FVector LaunchVelocity = MotionControllerComp->GetForwardVector() * TeleportProjectileSpeed;

		TArray<AActor*> IgnoreActors;
		SetIgnoreActorsForTeleportDestination(IgnoreActors);

		FPredictProjectilePathParams PredictProjectilePathParams(
			TeleportProjectileRadius,
			Start,
			LaunchVelocity,
			TeleportSimulationTime,
			ECollisionChannel::ECC_Visibility
		);

		PredictProjectilePathParams.bTraceComplex = bTeleportTraceComplex;
		PredictProjectilePathParams.ActorsToIgnore = IgnoreActors;
		FPredictProjectilePathResult Result;

		bool bHit = UGameplayStatics::PredictProjectilePath(this, PredictProjectilePathParams, Result);
		if (!bHit) return false;

		// Store positions of projectile path for OutPath param
		for (auto& PointData : Result.PathData)
		{
			OutPath.Add(PointData.Location);
		}

		// Get point on navmesh where PredictProjectilePath hit for OutLocation param
		FNavLocation NavLocation;
		bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->ProjectPointToNavigation(Result.HitResult.Location, NavLocation);
		if (!bOnNavMesh) return false;

		OutLocation = NavLocation.Location;

		return true;
	}

	return false;
}


void ADVRMotionController::SetIgnoreActorsForTeleportDestination(TArray<AActor*>& IgnoreActors)
{
	IgnoreActors.Add(this);
	if (CurrentGrabedActor)
	{
		IgnoreActors.Add(CurrentGrabedActor);
	}
	if (OtherHandMotionController && OtherHandMotionController->IsGrabbingActor())
	{
		IgnoreActors.Add(OtherHandMotionController->GetCurrentGrabbedActor());
	}
}


void ADVRMotionController::SetTeleportSplineMeshComponents(const TArray<FVector>& Path)
{
	if (TeleportSplinePath && TeleportArchMesh && TeleportArchMaterial)
	{
		// Add all points from teleport projectile path to TeleportSplinePath
		UpdateTeleportSpline(Path);

		// Hide all SplineMeshComponents in object pool
		for (auto SplineMesh : TeleportMeshObjectPool)
		{
			SplineMesh->SetVisibility(false);
		}

		int32 SegmentNumber = Path.Num() - 1;
		for (int32 i = 0; i < SegmentNumber; ++i)
		{
			// create a new SplineMeshComponent if there are not enough in TeleportMeshObjectPool
			if (TeleportMeshObjectPool.Num() <= i)
			{
				USplineMeshComponent* SplineMeshComponent = NewObject<USplineMeshComponent>(this);
				if (SplineMeshComponent)
				{
					SplineMeshComponent->SetMobility(EComponentMobility::Movable);
					SplineMeshComponent->AttachToComponent(TeleportSplinePath, FAttachmentTransformRules::KeepRelativeTransform);
					SplineMeshComponent->SetStaticMesh(TeleportArchMesh);
					SplineMeshComponent->SetMaterial(0, TeleportArchMaterial);
					SplineMeshComponent->RegisterComponent();

					TeleportMeshObjectPool.Add(SplineMeshComponent);
				}
			}

			// Add Mesh from TeleportMeshObjectPool to position on TeleportSplinePath
			if (TeleportMeshObjectPool[i])
			{
				FVector StartPos, StartTangent, EndPos, EndTangent;
				TeleportSplinePath->GetLocalLocationAndTangentAtSplinePoint(i, StartPos, StartTangent);
				TeleportSplinePath->GetLocalLocationAndTangentAtSplinePoint(i + 1, EndPos, EndTangent);
				TeleportMeshObjectPool[i]->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);

				TeleportMeshObjectPool[i]->SetVisibility(true, true);
			}
		}
	}
}


void ADVRMotionController::UpdateTeleportSpline(const TArray<FVector>& Path)
{
	if (TeleportSplinePath)
	{
		TeleportSplinePath->ClearSplinePoints(false);

		// Add points from the teleport projectile prediction path to TeleportSplinePath
		for (int32 i = 0; i < Path.Num(); ++i)
		{
			FVector LocalPosition = TeleportSplinePath->GetComponentTransform().InverseTransformPosition(Path[i]);
			FSplinePoint Point(i, LocalPosition, ESplinePointType::Curve);
			TeleportSplinePath->AddPoint(Point, false);
		}

		TeleportSplinePath->UpdateSpline();
	}
}


void ADVRMotionController::ClearTeleportPath()
{
	TArray<FVector> EmptyPath;
	SetTeleportSplineMeshComponents(EmptyPath);
}


void ADVRMotionController::ShowTeleportDestination(bool Show)
{
	if (TeleportDestinationMarker)
	{
		TeleportDestinationMarker->SetVisibility(Show, true);
	}
}


void ADVRMotionController::StartFindTeleportDestination()
{
	bLookForTeleportDestination = true;
}


void ADVRMotionController::StopFindTeleportDestination()
{
	bLookForTeleportDestination = false;
	ClearTeleportPath();
	ShowTeleportDestination(false);
}


bool ADVRMotionController::GetCurrentTeleportDestinationMarketLocation(FVector& Location)
{
	if (bLookForTeleportDestination && bHasValidTeleportDestination)
	{
		Location = TeleportDestination;
	}

	return bLookForTeleportDestination && bHasValidTeleportDestination;
}


/*******************************************************************/
/* Configuration */
/*******************************************************************/
void ADVRMotionController::SetHand(EControllerHand Hand)
{
	if (MotionControllerComp)
	{
		ControllerHand = Hand;
		MotionControllerComp->SetTrackingSource(ControllerHand);

		if (ControllerHand == EControllerHand::Left)
		{
			MeshComp->SetWorldScale3D(FVector(HandScale, HandScale, -HandScale));
			MeshComp->SetRelativeRotation(FRotator(0.f, 0.f, -270.f));
		}
		else
		{
			MeshComp->SetWorldScale3D(FVector(HandScale));
			MeshComp->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
		}
	}
}


void ADVRMotionController::SetControllerMode(EControllerMode Mode)
{
	if (Mode == EControllerMode::ECM_UI)
	{
		if (WidgetInteractionComp)
		{
			WidgetInteractionComp->SetActive(true);
		}

		ControllerMode = EControllerMode::ECM_UI;
	}
	else // ECM_Game
	{
		if (WidgetInteractionComp)
		{
			WidgetInteractionComp->SetActive(false);
		}

		ControllerMode = EControllerMode::ECM_Game;
	}
}


/*******************************************************************/
/* Input */
/*******************************************************************/
void ADVRMotionController::Grab()
{
	if (GrabState != EGrabState::EGS_Grab)
	{
		GrabState = EGrabState::EGS_Grab;

		bool AttachSuccess = TryAttachOverlappedActorToPhysicsHandle();
		if (AttachSuccess)
		{
			AlertGrabbedActorOfGrabState(EGrabState::EGS_Grab);

			// check if other motion controller is grabbing this actor. If so release Actor
			if (OtherHandMotionController && OtherHandMotionController->IsGrabbingActor() && OtherHandMotionController->GetCurrentGrabbedActor() == PreviousOverlappedPhysicsActor)
			{
				OtherHandMotionController->Release();
			}
		}
	}
}


void ADVRMotionController::Release()
{
	if (GrabState != EGrabState::EGS_Release)
	{
		GrabState = EGrabState::EGS_Release;

		if (CurrentGrabedActor)
		{
			AlertGrabbedActorOfGrabState(EGrabState::EGS_Release);
		}

		CurrentGrabedActor = nullptr;

		if (PhysicsConstraintComp)
		{
			PhysicsConstraintComp->BreakConstraint();
		}
	}
}


void ADVRMotionController::TriggerPulled()
{
	if (ControllerMode == EControllerMode::ECM_UI && WidgetInteractionComp)
	{
		WidgetInteractionComp->PressPointerKey(EKeys::LeftMouseButton);
	}
}


void ADVRMotionController::TriggerReleased()
{
	if (ControllerMode == EControllerMode::ECM_UI && WidgetInteractionComp)
	{
		WidgetInteractionComp->ReleasePointerKey(EKeys::LeftMouseButton);
	}
}

