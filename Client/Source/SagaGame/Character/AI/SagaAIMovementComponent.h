#pragma once
#include "SagaGame.h"
#include "SagaGameInfo.h"
#include <GameFramework/FloatingPawnMovement.h>

#include "SagaAIMovementComponent.generated.h"

UCLASS(BlueprintType, Category = "CandyLandSaga|Game|AI")
class SAGAGAME_API USagaAIMovementComponent : public UFloatingPawnMovement
{
	GENERATED_BODY()
	
};
