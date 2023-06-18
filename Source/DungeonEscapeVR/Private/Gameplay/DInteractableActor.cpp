// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/DInteractableActor.h"


// Engine Includes
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"


// Game Includes
#include "Player/DVRPlayerCharacter.h"


static const int32 ENABLE_OUTLINE_STENCIL = 2;
static const int32 DISABLE_OUTLINE_STENCIL = 0;


// Sets default values
ADInteractableActor::ADInteractableActor()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.f;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetEnableGravity(true);
	MeshComp->SetGenerateOverlapEvents(true);
	MeshComp->CanCharacterStepUpOn = ECB_No;
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComp->SetCollisionProfileName("PhysicsActor");
	
	InteractionAlertSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionAlertSphereComp"));
	InteractionAlertSphereComp->SetupAttachment(GetRootComponent());
	InteractionAlertSphereComp->SetGenerateOverlapEvents(true);
	InteractionAlertSphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionAlertSphereComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	InteractionAlertSphereComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	InteractionAlertSphereComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	
	InteractionAlertTrigger = nullptr;
	bOutlineEnabled = false;
	bPlayerCharacterTeleporting = false;
}


// Called when the game starts or when spawned
void ADInteractableActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (InteractionAlertSphereComp)
	{
		InteractionAlertSphereComp->OnComponentBeginOverlap.AddDynamic(this, &ADInteractableActor::OnInteractionAlertSphereCompBeginOverlap);
		InteractionAlertSphereComp->OnComponentEndOverlap.AddDynamic(this, &ADInteractableActor::OnInteractionAlertSphereCompEndOverlap);
	}

	BindPlayerPawnTeleportEvents();

	if (MeshComp)
	{
		MeshComp->SetMassOverrideInKg(NAME_None, Weight, true);
	}
}


void ADInteractableActor::BindPlayerPawnTeleportEvents()
{
	ADVRPlayerCharacter* VRPlayerCharacter = Cast<ADVRPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (VRPlayerCharacter)
	{
		VRPlayerCharacter->OnPlayerBeginTeleport.AddDynamic(this, &ADInteractableActor::OnPlayerBeingTeleport);
		VRPlayerCharacter->OnPlayerFinishTeleport.AddDynamic(this, &ADInteractableActor::OnPlayerFinishTeleport);
	}
}


void ADInteractableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ProcessShowMeshOutline();
}


/*******************************************************************/
/* Mesh Outline */
/*******************************************************************/
void ADInteractableActor::ProcessShowMeshOutline()
{
	if (InteractionAlertTrigger)
	{
		const bool UnobstructedView = UnobstructedViewToInteractionAlertTrigger();
		if (UnobstructedView && !bOutlineEnabled && !bPlayerCharacterTeleporting && !bIsPickedUp)
		{
			SetEnableMeshCompOutline(true);
		}
		else if ((!UnobstructedView && bOutlineEnabled) || bPlayerCharacterTeleporting || bIsPickedUp)
		{
			SetEnableMeshCompOutline(false);
		}
	}
	else if (bOutlineEnabled)
	{
		SetEnableMeshCompOutline(false);
	}
}


bool ADInteractableActor::UnobstructedViewToInteractionAlertTrigger() const
{
	bool UnobstructedView = true;
	if (InteractionAlertTrigger && InteractionAlertSphereComp)
	{
		TArray<AActor*> IgnoreActors;

		IgnoreActors.Add(InteractionAlertTrigger->GetOwner());
		IgnoreActors.Add(InteractionAlertSphereComp->GetOwner());
		FHitResult HitResult;

		UnobstructedView = !UKismetSystemLibrary::LineTraceSingle(
			GetWorld(),
			InteractionAlertSphereComp->GetComponentLocation(),
			InteractionAlertTrigger->GetComponentLocation(),
			UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
			false,
			IgnoreActors,
			EDrawDebugTrace::None,
			HitResult,
			true
		);
	}
	
	return UnobstructedView;
}


void ADInteractableActor::SetEnableMeshCompOutline(bool Enable)
{
	if (MeshComp)
	{
		MeshComp->SetRenderCustomDepth(Enable);

		const int32 StencilValue = Enable ? ENABLE_OUTLINE_STENCIL : DISABLE_OUTLINE_STENCIL;
		MeshComp->SetCustomDepthStencilValue(StencilValue);
	}

	bOutlineEnabled = Enable;
}


void ADInteractableActor::OnInteractionAlertSphereCompBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	InteractionAlertTrigger = OtherComp;
}


void ADInteractableActor::OnInteractionAlertSphereCompEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherComp == InteractionAlertTrigger)
	{
		InteractionAlertTrigger = nullptr;
	}
}


void ADInteractableActor::OnPlayerBeingTeleport()
{
	SetEnableMeshCompOutline(false);
	bPlayerCharacterTeleporting = true;
}


void ADInteractableActor::OnPlayerFinishTeleport()
{
	bPlayerCharacterTeleporting = false;
}


/*******************************************************************/
/* Interaction */
/*******************************************************************/
void ADInteractableActor::GrabActor()
{
	bIsPickedUp = true;
	SetEnableMeshCompOutline(false);
}


void ADInteractableActor::ReleaseActor()
{
	bIsPickedUp = false;
}

