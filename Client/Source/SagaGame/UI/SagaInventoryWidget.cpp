#include "UI/SagaInventoryWidget.h"
#include <Components/Button.h>
#include <Components/ListView.h>
#include <Blueprint/UserWidget.h>
#include <NiagaraFunctionLibrary.h>
#include <NiagaraComponent.h>

#include "InventoryItemData.h"
#include "SagaInventoryListWidget.h"
#include "Item/Gumball.h"
#include "PlayerControllers/SagaInGamePlayerController.h"

void
USagaInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	myCloseButton = Cast<UButton>(GetWidgetFromName(TEXT("CloseButton")));

	if (myCloseButton)
	{
		myCloseButton->OnClicked.AddDynamic(this, &USagaInventoryWidget::CloseButtonClick);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CloseButton not found!"));
	}

	myInventory = Cast<UListView>(GetWidgetFromName(TEXT("InventoryList")));
	if (myInventory)
	{
		myInventory->OnItemIsHoveredChanged().AddUObject(this, &USagaInventoryWidget::OnListItemHover);
		myInventory->OnItemSelectionChanged().AddUObject(this, &USagaInventoryWidget::OnListItemSelection);
		myInventory->OnItemClicked().AddUObject(this, &USagaInventoryWidget::OnListItemClick);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryList not found!"));
	}

	FString IconPath[3] =
	{
		TEXT("/Script/Engine.Texture2D'/Game/UI/Images/Tex_heart.Tex_heart'"),
		TEXT("/Script/Engine.Texture2D'/Game/UI/Images/gumball_machine.gumball_machine'"),
		TEXT("/Script/Engine.Texture2D'/Game/UI/Images/smoke_bomb.smoke_bomb'")
	};

	FString ItemName[3] =
	{
		TEXT("EnergyDrink"),
		TEXT("Gumball"),
		TEXT("SmokeBomb")
	};

	for (int32 i = 0; i < 10; ++i)
	{
		UInventoryItemData* ItemData = NewObject<UInventoryItemData>();

		int32 IconIndex = FMath::RandRange(0, 2);
		UTexture2D* IconTexture = LoadObject<UTexture2D>(nullptr, *IconPath[IconIndex]);

		ItemData->SetInfo(IconTexture, ItemName[IconIndex], 1);

		UE_LOG(LogTemp, Warning, TEXT("Created item: %s"), *ItemName[IconIndex]);  // ������ ���� �α�

		myInventory->AddItem(ItemData);
	}
}

void USagaInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void USagaInventoryWidget::CloseButtonClick()
{
	UE_LOG(LogTemp, Warning, TEXT("CloseButton clicked"));
	SetVisibility(ESlateVisibility::Collapsed);

	APlayerController* Controller = GetOwningPlayer();
	if (Controller)
	{
		ASagaInGamePlayerController* Character = Cast<ASagaInGamePlayerController>(Controller);
		if (Character)
		{
			Character->SetInventoryVisibility(false);
		}
	}
}

void USagaInventoryWidget::OnListItemHover(UObject* Item, bool IsHovered)
{
	UInventoryItemData* ItemData = Cast<UInventoryItemData>(Item);

	if (IsValid(ItemData) && mSelectionItem != ItemData)
	{
		ItemData->SetMouseOn(IsHovered);

		USagaInventoryListWidget* EntryWidget = myInventory->GetEntryWidgetFromItem<USagaInventoryListWidget>(Item);
		if (IsValid(EntryWidget))
		{
			if (IsHovered)
			{
				EntryWidget->SetMouseState(EEntryWidgetMouseState::MouseOn);
			}
			else
			{
				EntryWidget->SetMouseState(EEntryWidgetMouseState::None);
			}
		}
	}
}

void USagaInventoryWidget::OnListItemSelection(UObject* Item)
{
	if (IsValid(mSelectionItem))
	{
		mSelectionItem->SetSelect(false);
		USagaInventoryListWidget* EntryWidget = myInventory->GetEntryWidgetFromItem<USagaInventoryListWidget>(mSelectionItem);
		if (IsValid(EntryWidget))
		{
			EntryWidget->SetMouseState(EEntryWidgetMouseState::None);
		}
	}

	mSelectionItem = Cast<UInventoryItemData>(Item);

	if (IsValid(mSelectionItem))
	{
		mSelectionItem->SetSelect(true);
		USagaInventoryListWidget* EntryWidget = myInventory->GetEntryWidgetFromItem<USagaInventoryListWidget>(mSelectionItem);
		if (IsValid(EntryWidget))
		{
			EntryWidget->SetMouseState(EEntryWidgetMouseState::Select);
		}
	}
}

void USagaInventoryWidget::OnListItemClick(UObject* Item)
{
	UInventoryItemData* ItemData = Cast<UInventoryItemData>(Item);

	if (IsValid(ItemData))
	{
		FString ItemName = ItemData->GetItemName();
		UE_LOG(LogTemp, Warning, TEXT("Item clicked: %s"), *ItemName);

		if (ItemName == "EnergyDrink")
		{
			UseEnergyDrink();
		}
		else if (ItemName == "Gumball")
		{
			UseGumball();
		}
		else if (ItemName == "SmokeBomb")
		{
			UseSmokeBomb();
		}

		myInventory->RemoveItem(ItemData);
	}
}

void USagaInventoryWidget::UseEnergyDrink()
{
	UE_LOG(LogTemp, Warning, TEXT("Using Energy Drink"));

	//�÷��̾� HP ȸ�� ���� �߰�
	// + �÷��̾� HPȸ�� �� ����Ʈ ��� �߰�


}

void USagaInventoryWidget::UseGumball()
{
	UE_LOG(LogTemp, Warning, TEXT("Using Gumball"));

	APlayerController* PlayerController = GetOwningPlayer();
	if (PlayerController)
	{
		APawn* PlayerPawn = PlayerController->GetPawn();
		if (PlayerPawn)
		{
			FVector SpawnLocation = PlayerPawn->GetActorLocation() + PlayerPawn->GetActorForwardVector() * 200.0f;
			FRotator SpawnRotation = PlayerPawn->GetActorRotation();
			FActorSpawnParameters SpawnParams;
			GetWorld()->SpawnActor<AGumball>(AGumball::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
		}
	}
}

void USagaInventoryWidget::UseSmokeBomb()
{
	UE_LOG(LogTemp, Warning, TEXT("Using Smoke Bomb"));
	APlayerController* PlayerController = GetOwningPlayer();
	if (PlayerController)
	{
		APawn* PlayerPawn = PlayerController->GetPawn();
		if (PlayerPawn)
		{
			FVector SpawnLocation = PlayerPawn->GetActorLocation() + PlayerPawn->GetActorForwardVector() * 50.0f;
			SpawnLocation.Z += 200.0f;  // 200 unit above player

			FRotator SpawnRotation = PlayerPawn->GetActorRotation();
			UNiagaraSystem* SmokeEffect = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Script/Niagara.NiagaraSystem'/Game/Item/VFX/NS_Smoke.NS_Smoke'"));
			if (SmokeEffect)
			{
				UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SmokeEffect, SpawnLocation, SpawnRotation);
				if (NiagaraComponent)
				{
					NiagaraComponent->SetAutoDestroy(true);  // auto destroy after effect is done
				}
			}
		}
	}

}
