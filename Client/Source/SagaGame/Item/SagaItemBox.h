// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../SagaGameInfo.h"
#include "GameFramework/Actor.h"
#include "SagaItemBox.generated.h"

UCLASS()
class SAGAGAME_API ASagaItemBox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASagaItemBox();

protected:
	UPROPERTY(VisibleAnywhere, Category = Box)
	TObjectPtr<class UBoxComponent> Trigger;

	UPROPERTY(VisibleAnywhere, Category = Box)
	TObjectPtr<class UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category = Effect)
	TObjectPtr<class UParticleSystemComponent> Effect;

	UPROPERTY(EditAnywhere, Category = Item)
	TObjectPtr<class USagaWeaponData> Weapon;

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEffectFinished(class UParticleSystemComponent* ParticleSystem);
};