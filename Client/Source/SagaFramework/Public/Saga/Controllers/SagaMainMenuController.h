#pragma once
#include "Saga/Controllers/SagaInterfaceController.h"

#include "SagaMainMenuController.generated.h"

UCLASS(BlueprintType, Category = "CandyLandSaga|System|Controller")
class SAGAFRAMEWORK_API ASagaMainMenuController : public ASagaInterfaceController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
