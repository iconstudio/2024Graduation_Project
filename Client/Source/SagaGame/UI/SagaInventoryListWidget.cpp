// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SagaInventoryListWidget.h"
#include "InventoryItemData.h"



void USagaInventoryListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	mIconImage = Cast<UImage>(GetWidgetFromName(TEXT("Icon")));
	mSelection = Cast<UImage>(GetWidgetFromName(TEXT("Selection")));
	//mNameText = Cast<UTextBlock>(GetWidgetFromName(TEXT("NameText")));
	mCountText = Cast<UTextBlock>(GetWidgetFromName(TEXT("CountText")));

	if (!mSelection)
	{
		UE_LOG(LogTemp, Warning, TEXT("Selection widget not found!"));
	}
}

void USagaInventoryListWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void USagaInventoryListWidget::SetMouseState(EEntryWidgetMouseState State)
{
	switch (State)
	{
	case EEntryWidgetMouseState::None:
		mSelection->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.f));
		break;
	case EEntryWidgetMouseState::MouseOn:
		mSelection->SetColorAndOpacity(FLinearColor(1.f, 1.f, 0.f, 0.3f));
		break;
	case EEntryWidgetMouseState::Select:
		mSelection->SetColorAndOpacity(FLinearColor(0.f, 0.f, 1.f, 0.3f));
		break;
	}
}

//�������̽��� �߰��ϸ� ������ OnListItemObjectSet�Լ����� �� �Լ��� ȣ�����ְ� ����.
//����Ʈ�� ����� ���ŵǸ� ���ŵ� ��Ͽ� �߰��� ������ object�� ���ڷ� ����. ����Ʈ�� �߰��� �����͸� ��ü�� �����Ѵٴ� ��.
//ó�� ������ ��, Construct �Լ����� ȣ�� --> �״����� SetItem�Լ� ȣ��
void USagaInventoryListWidget::SetItem(UObject* Item)
{
	UInventoryItemData* ItemData = Cast<UInventoryItemData>(Item);

	if (IsValid(ItemData))
	{
		mData = ItemData;

		mIconImage->SetBrushFromTexture(ItemData->GetIconTexture());
		//mNameText->SetText(FText::FromString(ItemData->GetItemName()));
		mCountText->SetText(FText::FromString(FString::FromInt(ItemData->GetCount())));

		if (mData->IsSelect())
		{
			SetMouseState(EEntryWidgetMouseState::Select);
		}

		else if (mData->IsMouseOn())
		{
			SetMouseState(EEntryWidgetMouseState::MouseOn);
		}

		else
		{
			SetMouseState(EEntryWidgetMouseState::None);
		}
	}
}
