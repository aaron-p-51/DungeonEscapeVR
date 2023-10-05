// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/DCellDoor.h"

// Engine Includes
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"


// Game Includes
#include "DGameModeBase.h"
#include "Gameplay/DCellDoorTrigger.h"


// Sets default values
ADCellDoor::ADCellDoor()
{
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootComp);

	CellDoorStaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CellDoorStaticMesh"));
	CellDoorStaticMeshComp->SetupAttachment(GetRootComponent());

	CellDoorBlockingCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BlockingBoxComp"));
	CellDoorBlockingCollision->SetupAttachment(GetRootComponent());
	CellDoorBlockingCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	CellDoorBlockingCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CellDoorBlockingCollision->SetCanEverAffectNavigation(true);

	CellDoorState = ECellDoorState::ECDS_Closed;

	bDebugForceGateOpen = false;
	bGameModeForceAllGatesOpen = false;

	PrimaryActorTick.bCanEverTick = true;
}


// Called when the game starts or when spawned
void ADCellDoor::BeginPlay()
{
	Super::BeginPlay();

	if (CellDoorStaticMeshComp)
	{
		InitialCellDoorHeight = CellDoorStaticMeshComp->GetComponentLocation().Z;
	}

#if !UE_BUILD_SHIPPING

	if (ADGameModeBase* GameModeBase = GetWorld()->GetAuthGameMode<ADGameModeBase>())
	{
		bGameModeForceAllGatesOpen = GameModeBase->bForceAllCellDoorsOpen;
	}

#endif
}


// Called every frame
void ADCellDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ProcessDoorOpenCloseState();
}


void ADCellDoor::ProcessDoorOpenCloseState()
{
	// Cell door will always complete the process of opening or closing
	if (CellDoorState == ECellDoorState::ECDS_Opening || CellDoorState == ECellDoorState::ECDS_Closing) return;

	float Weight = 0.f;

#if !UE_BUILD_SHIPPING

	if (bGameModeForceAllGatesOpen || bDebugForceGateOpen)
	{
		Weight = FLT_MAX;
	}
	else
	{
		Weight = CalculateTotalWeightOnTriggers();
	}

#else

	Weight = CalculateTotalWeightOnTriggers();

#endif

	if (CellDoorState == ECellDoorState::ECDS_Closed && Weight >= WeightToOpenCell)
	{
		OpenCellDoor();
	}
	else if (CellDoorState == ECellDoorState::ECDS_Opened && Weight < WeightToOpenCell)
	{
		CloseCellDoor();
	}
}


float ADCellDoor::CalculateTotalWeightOnTriggers() const
{
	float WeightOnTriggers = 0.f;
	for (const auto& Trigger : CellDoorTriggers)
	{
		WeightOnTriggers += Trigger->GetWeightOnTrigger();
	}

	return WeightOnTriggers;
}


/*******************************************************************/
/* Cell Door Open/Close */
/*******************************************************************/
void ADCellDoor::OpenCellDoor()
{
	CellDoorState = ECellDoorState::ECDS_Opening;
	OnCellDoorStateChange.Broadcast(this, CellDoorState);

	BP_OpenCellDoor();
}


void ADCellDoor::CloseCellDoor()
{
	CellDoorState = ECellDoorState::ECDS_Closing;
	OnCellDoorStateChange.Broadcast(this, CellDoorState);

	// Block player and PhysicsBodies (CellDoorKeys) from passing through the cell door
	if (CellDoorBlockingCollision)
	{
		CellDoorBlockingCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECollisionResponse::ECR_Block);
		CellDoorBlockingCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
		CellDoorBlockingCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block);
		CellDoorBlockingCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		CellDoorBlockingCollision->SetCanEverAffectNavigation(true);
	}

	BP_CloseCellDoor();
}


void ADCellDoor::OnUpdateCellDoorHeight(float CellDoorHeightOffset)
{
	if (!CellDoorStaticMeshComp) return;

	// Change CellDoor height based on CellDoorHeightOffset in relation to InitialCellDoorHeight
	const FVector CurrentCellDoorLocation = CellDoorStaticMeshComp->GetComponentLocation();
	const float NewCellDoorHeight = CellDoorHeightOffset + InitialCellDoorHeight;
	const FVector NewCellDoorLocation = FVector(CurrentCellDoorLocation.X, CurrentCellDoorLocation.Y, NewCellDoorHeight);

	CellDoorStaticMeshComp->SetWorldLocation(NewCellDoorLocation);
}


void ADCellDoor::OnFinishCellDoorOpened()
{
	// Allow player and PhysicsBodies (CellDoorKeys) to pass through the cell door
	if (CellDoorBlockingCollision)
	{
		CellDoorBlockingCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECollisionResponse::ECR_Ignore);
		CellDoorBlockingCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		CellDoorBlockingCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		CellDoorBlockingCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
		CellDoorBlockingCollision->SetCanEverAffectNavigation(false);
	}

	CellDoorState = ECellDoorState::ECDS_Opened;
	OnCellDoorStateChange.Broadcast(this, CellDoorState);
}


void ADCellDoor::OnFinishedCellDoorClosed()
{
	CellDoorState = ECellDoorState::ECDS_Closed;
	OnCellDoorStateChange.Broadcast(this, CellDoorState);
}
