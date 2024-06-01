// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AI/SagaMonsterAIPawn.h"
#include "SagaMonsterAIController.h"

ASagaMonsterAIPawn::ASagaMonsterAIPawn()
{
	AIControllerClass = ASagaMonsterAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    mCapsule->SetCollisionProfileName(TEXT("Enemy"));
}

void ASagaMonsterAIPawn::BeginPlay()
{
	Super::BeginPlay();

    // �������� ���� ������ Ȯ��
    if (!HasAuthority())
    {
        return;
    }

    // AI�� �������� �����Ǹ� �ڵ� ���Ǹ� Ȱ��ȭ
    //AutoPossessAI = EAutoPossessAI::Spawned;

    // �ʿ��� ��� �⺻ ��Ʈ�ѷ��� ����
    /*if (AIControllerClass != nullptr && GetController() == nullptr)
    {
        SpawnDefaultController();
    }*/
}

void ASagaMonsterAIPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
