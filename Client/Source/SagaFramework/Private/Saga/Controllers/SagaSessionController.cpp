#include "Saga/Controllers/SagaSessionController.h"

void
ASagaSessionController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeUIOnly mode{};
	SetInputMode(mode);
}