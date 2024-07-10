﻿#include "SagaItemBox.h"
#include <Kismet/GameplayStatics.h>

#include "Interface/SagaCharacterItemInterface.h"

ASagaItemBox::ASagaItemBox()
	: Super()
{
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Effect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Effect"));

	RootComponent = Trigger;

	Mesh->SetupAttachment(Trigger);
	Effect->SetupAttachment(Trigger);

	Trigger->SetBoxExtent(FVector(30.f, 30.f, 30.f));
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ASagaItemBox::OnOverlapBegin);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BoxMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/Item/ItemBox/Box_Prop.Box_Prop'"));
	if (BoxMeshRef.Object)
	{
		Mesh->SetStaticMesh(BoxMeshRef.Object);
	}

	Mesh->SetRelativeLocation(FVector(0.f, -3.5f, -30.0f));
	Mesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 1.5f));
	Mesh->SetCollisionProfileName(TEXT("NoCollision"));

	static ConstructorHelpers::FObjectFinder<UParticleSystem> EffectRef(TEXT(""));
	if (EffectRef.Object)
	{
		Effect->SetTemplate(EffectRef.Object);
		Effect->bAutoActivate = false;
	}

	Trigger->SetCollisionProfileName(TEXT("Item"));

	// Make Mesh Collision Profile to NoCollision because the TriggerBox has the Item Collision Profile
	Mesh->SetCollisionProfileName(TEXT("NoCollision"));
}

void
ASagaItemBox::BeginPlay()
{
	Super::BeginPlay();

	// TODO: 박스가 월드에 스폰될 때 바로 아이템 유형 설정해야함
	SetRandomItemType();
}

void
ASagaItemBox::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ISagaCharacterItemInterface* OverlappingPawn = Cast<ISagaCharacterItemInterface>(OtherActor);
	if (OverlappingPawn)
	{
		OverlappingPawn->TakeItem(storedItemType);
		Effect->Activate(true);
		Mesh->SetHiddenInGame(true);
		SetActorEnableCollision(false);

		Effect->OnSystemFinished.AddDynamic(this, &ASagaItemBox::OnEffectFinished);
	}
	else
	{
		Destroy();
	}
}

void
ASagaItemBox::SetRandomItemType()
{
	int32 RandomIndex = FMath::RandRange(0, 2);

	storedItemType = static_cast<ESagaItemTypes>(RandomIndex);
}

void
ASagaItemBox::OnEffectFinished(UParticleSystemComponent* ParticleSystem)
{
	Destroy();
}
