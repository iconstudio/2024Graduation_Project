// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "EngineGlobals.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "SagaSpawnPoint.generated.h"

UENUM(BlueprintType)
enum class ESpawnType : uint8
{
	Normal, //������ ��ü�� ���ŵǸ� ������ �ð��� ������ ���� ->ĳ���Ϳ� �������� ���
	Once,	//�ѹ����� �� ��������Ʈ ���� ->AI�� ���
	Loop   //������ Ƚ����ŭ ����
};

UCLASS()
class SAGAGAME_API ASagaSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASagaSpawnPoint();

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> mRoot;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<UArrowComponent> mArrowComponent;
#endif


	//�����Ϸ��� Ŭ���� ���� ����
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> mSpawnClass;

	//���� Ŭ���� �̿��� ������ ���� ��ü�� �޸��ּ� ����
	TObjectPtr<AActor> mSpawnActor;

	UPROPERTY(EditAnywhere)
	ESpawnType mSpawnType;


	//�����ð� 0�̸� ��� ����
	UPROPERTY(EditAnywhere)
	float mSpawnTime = 0.f;

	float mSpawnAccTime;

	//ī��Ʈ �⺻���� -1�� ��������(���ѻ���)
	UPROPERTY(EditAnywhere)
	int32 mCount = -1;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void Spawn();
};
