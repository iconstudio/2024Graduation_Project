// Fill out your copyright notice in the Description page of Project Settings.


#include "SagaCharacterSelectWidget.h"

void USagaCharacterSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	mStartButton = Cast<UButton>(GetWidgetFromName(TEXT("Start")));
	mStartButton->OnClicked.AddDynamic(this, &USagaCharacterSelectWidget::StartButtonClick);
}

void USagaCharacterSelectWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void USagaCharacterSelectWidget::StartButtonClick()
{

	//서버로 보내는 함수 추가



	UGameplayStatics::OpenLevel(GetWorld(), TEXT("SagaGameLevel"));
}

void USagaCharacterSelectWidget::StartButtonEnable(bool bEnable)
{
	mStartButton->SetIsEnabled(bEnable);
}
