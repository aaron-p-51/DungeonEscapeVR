// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/DEscapeSuccessVolume.h"


// Engine Includes
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"


// Game Includes
#include "DGameModeBase.h"
#include "Player/DVRPlayerCharacter.h"


// Sets default values
ADEscapeSuccessVolume::ADEscapeSuccessVolume()
{
	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	SetRootComponent(BoxComp);
	BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BoxComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	
	SuccessWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("SuccessWidgetComp"));
	SuccessWidgetComp->SetupAttachment(GetRootComponent());
}


// Called when the game starts or when spawned
void ADEscapeSuccessVolume::BeginPlay()
{
	Super::BeginPlay();

	if (BoxComp)
	{
		BoxComp->OnComponentBeginOverlap.AddDynamic(this, &ADEscapeSuccessVolume::OnBoxCompBeginOverlap);
		BoxComp->OnComponentEndOverlap.AddDynamic(this, &ADEscapeSuccessVolume::OnBoxCompEndOverlap);
	}
	
	if (SuccessWidgetComp)
	{
		SuccessWidgetComp->SetVisibility(false, true);
	}
}


void ADEscapeSuccessVolume::OnBoxCompBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ADVRPlayerCharacter* PlayerCharacter = Cast<ADVRPlayerCharacter>(OtherActor))
	{
		ShowSuccessWidget();
		InformGameModeEscapeSuccess();
	}
}


void ADEscapeSuccessVolume::OnBoxCompEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ADVRPlayerCharacter* PlayerCharacter = Cast<ADVRPlayerCharacter>(OtherActor))
	{
		InformGameModePlayerLeftEscapeArea();
	}
}


void ADEscapeSuccessVolume::InformGameModeEscapeSuccess()
{
	if (ADGameModeBase* GameMode = Cast<ADGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GameMode->PlayerEscapeSuccess();
	}
}


void ADEscapeSuccessVolume::InformGameModePlayerLeftEscapeArea()
{
	if (ADGameModeBase* GameMode = Cast<ADGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GameMode->PlayerLeftEscapeSuccessArea();
	}
}


void ADEscapeSuccessVolume::ShowSuccessWidget()
{
	if (SuccessWidgetComp)
	{
		SuccessWidgetComp->SetVisibility(true, true);
	}
}

