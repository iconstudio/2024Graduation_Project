// Fill out your copyright notice in the Description page of Project Settings.


#include "AIPawn/SagaBearPawn.h"


ASagaBearPawn::ASagaBearPawn()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("")); //���۷��� ����

	if (MeshAsset.Succeeded())
	{
		mMesh->SetSkeletalMesh(MeshAsset.Object);
	}

	mCapsule->SetCapsuleHalfHeight(85.f);
	mCapsule->SetCapsuleRadius(30.f);

	mMesh->SetRelativeLocation(FVector(0.f, 0.f, -85.f));
	mMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimAsset(TEXT("")); //���۷��� ������ ���� _C ���̱�
	if(AnimAsset.Succeeded())
	{
		mMesh->SetAnimInstanceClass(AnimAsset.Class);
	}

}

void ASagaBearPawn::BeginPlay()
{
	Super::BeginPlay();
}

void ASagaBearPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
