#include "Obstacles/MapObstacle1.h"
#include <Components/StaticMeshComponent.h>
#include <GameFramework/Controller.h>

#include "Item/SagaItemBox.h"

AMapObstacle1::AMapObstacle1()
	: Super()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// Initialize HPComponent
	HPComponent = CreateDefaultSubobject<UObstacleHPComponent>(TEXT("HPComponent"));

	// Set initial HP
	HPComponent->SetObjectHealth(50.0f);
}

void
AMapObstacle1::BeginPlay()
{
	Super::BeginPlay();

	if (HPComponent)
	{
		health = HPComponent->GetCurrentHealth();
	}
}

void
AMapObstacle1::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float
AMapObstacle1::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageApplied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);


	health -= DamageApplied;

	if (HPComponent)
	{
		HPComponent->TakeDamage(DamageApplied);
	}

	return DamageApplied;
}

void
AMapObstacle1::SpawnItemBox()
{
	FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 40.0f); // Adjust the z value if needed
	FRotator SpawnRotation = FRotator::ZeroRotator;
	FActorSpawnParameters SpawnParameters;

	ASagaItemBox* SpawnedItemBox = GetWorld()->SpawnActor<ASagaItemBox>(ASagaItemBox::StaticClass(), SpawnLocation, SpawnRotation, SpawnParameters);

	if (SpawnedItemBox)
	{
		UStaticMeshComponent* ItemMesh = SpawnedItemBox->GetMesh();

		if (ItemMesh)
		{
			ItemMesh->SetSimulatePhysics(true);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

			//ItemMesh->SetCollisionProfileName(TEXT("Item")); // Ensure it has a suitable collision profile
			FVector LaunchVelocity = FVector(0.0f, 0.0f, 500.0f); // z value for the popping-up effect
			ItemMesh->AddImpulse(LaunchVelocity, NAME_None, true);
		}
	}

	// Destroy the obstacle
	Destroy();
}
