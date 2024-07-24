﻿#include "SagaItemBox.h"
#include <Math/UnrealMathUtility.h>
#include <Templates/Casts.h>
#include <Kismet/GameplayStatics.h>

#include "Item/SagaWeaponData.h"
#include "Interface/SagaCharacterItemInterface.h"

#include "Saga/Network/SagaNetworkSubSystem.h"

ASagaItemBox::ASagaItemBox()
	: Super()
	, myItemId(-1)
{
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	RootComponent = Trigger;

	Mesh->SetupAttachment(Trigger);

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

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> NiagaraEffect(TEXT("/Script/Niagara.NiagaraSystem'/Game/VFX/VFX_Get/NS_Get.NS_Get'"));
	if (NiagaraEffect.Succeeded())
	{
		Effect = NiagaraEffect.Object;
		UE_LOG(LogTemp, Warning, TEXT("Item Niagara Effect Loaded"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Item Niagara Effect Not Loaded"));
	}

	Trigger->SetCollisionProfileName(TEXT("Item"));

	// Make Mesh Collision Profile to NoCollision because the TriggerBox has the Item Collision Profile
	Mesh->SetCollisionProfileName(TEXT("NoCollision"));

	isGrabbed = false;
}

void
ASagaItemBox::SetItemId(int32 id)
noexcept
{
	myItemId = id;
}

void
ASagaItemBox::BeginPlay()
{
	Super::BeginPlay();
}

void
ASagaItemBox::OnOverlapBegin(UPrimitiveComponent* component, AActor* other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool sweep, const FHitResult& SweepResult)
{
	ISagaCharacterItemInterface* pawn = Cast<ISagaCharacterItemInterface>(other);

	if (pawn)
	{
		const auto net = USagaNetworkSubSystem::GetSubSystem(GetWorld());

		if (net->IsOfflineMode())
		{
			pawn->TakeItem(static_cast<ESagaItemTypes>(FMath::RandRange(0, 2)));
		}
		else
		{
			// TODO
			net->SendRpcPacket(ESagaRpcProtocol::RPC_GRAB_ITEM, 0, myItemId);
		}

		if (Effect)
		{
			FVector SpawnLocation = GetActorLocation();
			FRotator SpawnRotation = GetActorRotation();
			UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Effect, SpawnLocation, SpawnRotation);

			if (NiagaraComponent)
			{
				FTimerHandle NiagaraTimerHandle;
				GetWorldTimerManager().SetTimer(NiagaraTimerHandle, NiagaraComponent, &UNiagaraComponent::Deactivate, 3.0f, false);
			}
		}

		Mesh->SetHiddenInGame(true);
		SetActorEnableCollision(false);

	}
	else
	{
		//Destroy();
	}
}

void
ASagaItemBox::OnEffectFinished(UParticleSystemComponent* ParticleSystem)
{
	Destroy();
}
