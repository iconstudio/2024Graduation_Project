#include "Saga/Level/SagaMainMenuLevel.h"
#include <Internationalization/Internationalization.h>
#include <UObject/Object.h>
#include <UObject/ConstructorHelpers.h>
#include <Delegates/Delegate.h>

#include "Saga/Interface/SagaMainMenuUiWidget.h"
#include "Saga/Interface/SagaUiPopup.h"

#include "Saga/Network/SagaNetworkSubSystem.h"

FText GetDescription(const ESagaConnectionContract& contract);

ASagaMainMenuLevel::ASagaMainMenuLevel()
noexcept
	: Super()
	, levelUiClass(), levelUiInstance()
	, errorPopupWidgetClass(), errorPopupWidgetInstance()
{
	SetNextLevelName(TEXT("LobbyLevel"));

	static ConstructorHelpers::FClassFinder<USagaMainMenuUiWidget> widget_class(TEXT("/Game/UI/Level/SagaMainMenuLevelUI.SagaMainMenuLevelUI_C"));

	if (widget_class.Succeeded())
	{
		levelUiClass = widget_class.Class;
	}
	else
	{
		UE_LOG(LogSagaFramework, Error, TEXT("[ASagaMainMenuController][Ctor] Could not find the class of user interface for main menu."));
	}

	static ConstructorHelpers::FClassFinder<USagaUiPopup> popup_class(TEXT("/Game/UI/Components/SagaUiPopup.SagaUiPopup_C"));

	if (popup_class.Succeeded())
	{
		errorPopupWidgetClass = popup_class.Class;
	}
	else
	{
		UE_LOG(LogSagaFramework, Fatal, TEXT("[ASagaMainMenuLevel][Ctor] Could not find the class of message popup for main menu."));
	}
}

void
ASagaMainMenuLevel::BeginPlay()
{
	Super::BeginPlay();

	levelUiInstance = CreateWidget<USagaMainMenuUiWidget>(GetWorld(), levelUiClass);
	if (IsValid(levelUiInstance))
	{
		levelUiInstance->AddToViewport(1);
		levelUiInstance->OpenNow();
	}
	else
	{
		UE_LOG(LogSagaFramework, Fatal, TEXT("[ASagaMainMenuController][BeginPlay] Could not create user interface for main menu."));
	}

	auto system = USagaNetworkSubSystem::GetSubSystem(GetWorld());

	system->OnFailedToConnect.AddDynamic(this, &ASagaMainMenuLevel::OnFailedToConnect);
	system->OnSignedIn.AddDynamic(this, &ASagaMainMenuLevel::OnSignedIn);
}

void
ASagaMainMenuLevel::OnFailedToConnect(ESagaConnectionContract reason)
{
	const auto reason_text = GetDescription(reason);
	const auto text = FString::Printf(TEXT("원인 '%s' 으로 인해 서버에 연결할 수 없습니다."), *reason_text.ToString());

	errorPopupWidgetInstance = CreateWidget<USagaUiPopup>(GetWorld(), errorPopupWidgetClass);

	if (IsValid(errorPopupWidgetInstance))
	{
		errorPopupWidgetInstance->AddToViewport(10);
		errorPopupWidgetInstance->SetTitle(FText::FromString(TEXT("연결 오류")));
		// C71017FF
		errorPopupWidgetInstance->SetTitleColorAndOpacity(FLinearColor{ 0.783538f, 0.06301f, 0.090842f });
		errorPopupWidgetInstance->SetText(FText::FromString(text));

		errorPopupWidgetInstance->Open();
	}
	else
	{
		UE_LOG(LogSagaFramework, Error, TEXT("[ASagaMainMenuLevel][OnFailedToConnect] Could not create an error popup for main menu."));
	}
}

void
ASagaMainMenuLevel::OnSignedIn(int32 my_id, const FName& nickname)
{
	GotoNextLevel();
}

FText
GetDescription(const ESagaConnectionContract& contract)
{
	switch (contract)
	{
	case ESagaConnectionContract::Success:
	{
		return FText::FromString(TEXT("성공"));
	}
	break;

	case ESagaConnectionContract::NoSocket:
	{
		return FText::FromString(TEXT("소켓 없음"));
	}
	break;

	case ESagaConnectionContract::SignInFailed:
	{
		return FText::FromString(TEXT("로그인 실패"));
	}
	break;

	case ESagaConnectionContract::CannotCreateWorker:
	{
		return FText::FromString(TEXT("작업자 스레드 생성 실패"));
	}
	break;

	case ESagaConnectionContract::ConnectError:
	{
		return FText::FromString(TEXT("연결 문제"));
	}
	break;

	case ESagaConnectionContract::WrongAddress:
	{
		return FText::FromString(TEXT("잘못된 주소"));
	}
	break;

	case ESagaConnectionContract::OtherError:
	{
		return FText::FromString(TEXT("기타 오류"));
	}
	break;

	case ESagaConnectionContract::SubSystemError:
	{
		return FText::FromString(TEXT("네트워크 하위 시스템 오류"));
	}
	break;

	default:
		break;
	}

	return FText();
}
