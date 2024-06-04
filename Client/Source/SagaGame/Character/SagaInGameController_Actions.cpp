#include "SagaInGamePlayerController.h"
#include <Logging/LogMacros.h>
#include <Containers/UnrealString.h>
#include <UObject/Object.h>
#include <InputActionValue.h>

#include "Character/SagaCharacterBase.h"

#include "Saga/Network/SagaRpcProtocol.h"
#include "Saga/Network/SagaNetworkSubSystem.h"

void
PrintVector(const FVector& vector)
{
	const FString str = vector.ToString();
	//UE_LOG(LogSagaGame, Log, TEXT("[Local][Controller] Movement Vector: (%s)"), *str);
}

void
ASagaInGamePlayerController::TriggerRideNPC(const FInputActionValue& Value)
{
	ASagaPlayableCharacter* ControlledCharacter = GetPawn<ASagaPlayableCharacter>();
	UE_LOG(LogSagaGame, Warning, TEXT("TriggerRideNPC"));

	if (ControlledCharacter)
	{
		ControlledCharacter->RideNPC();
	}
	else
	{
		UE_LOG(LogSagaGame, Warning, TEXT("This Character is not ASagaPlayableCharacter Type."));
	}
}

void
ASagaInGamePlayerController::RideNPCCallFunction()
{
	ASagaPlayableCharacter* ControlledCharacter = GetPawn<ASagaPlayableCharacter>();

	UE_LOG(LogSagaGame, Warning, TEXT("TriggerRideNPC"));
	if (ControlledCharacter)
	{
		ControlledCharacter->RideNPC();
	}
	else
	{
		UE_LOG(LogSagaGame, Warning, TEXT("This Character is not ASagaPlayableCharacter Type."));
	}
}

void
ASagaInGamePlayerController::BeginForwardWalk(const FInputActionValue& input)
{
	//UE_LOG(LogSagaGame, Log, TEXT("[Local][Controller] Begin Walking Straight"));

	auto dir = input.Get<FVector>();

	walkDirection.Y = input.Get<FVector>().Y;
	PrintVector(walkDirection);

	const auto system = USagaNetworkSubSystem::GetSubSystem(GetWorld());

	if (system->IsOfflineMode())
	{
		const auto character = GetPawn<ASagaCharacterBase>();

		character->ExecuteStraightWalk(static_cast<int>(walkDirection.Y));
		character->ExecuteStrafeWalk(static_cast<int>(walkDirection.X));
	}
	else
	{
		system->SendRpcPacket(ESagaRpcProtocol::RPC_BEG_WALK, walkDirection.X, walkDirection.Y);
	}
}

void
ASagaInGamePlayerController::MidForwardWalk(const FInputActionValue& input)
{
	if (walkDirection.Y != input.Get<FVector>().Y)
	{
		BeginForwardWalk(input);
	}
}

void
ASagaInGamePlayerController::EndForwardWalk(const FInputActionValue& input)
{
	//UE_LOG(LogSagaGame, Log, TEXT("[Local][Controller] End Walking Straight"));

	walkDirection.Y = input.Get<FVector>().Y;
	PrintVector(walkDirection);

	const auto system = USagaNetworkSubSystem::GetSubSystem(GetWorld());

	if (system->IsOfflineMode())
	{
		const auto character = GetPawn<ASagaCharacterBase>();

		character->ExecuteStraightWalk(static_cast<int>(walkDirection.Y));
		character->ExecuteStrafeWalk(static_cast<int>(walkDirection.X));
	}
	else
	{
		system->SendRpcPacket(ESagaRpcProtocol::RPC_END_WALK, walkDirection.X, walkDirection.Y);
	}
}

void
ASagaInGamePlayerController::BeginStrafeWalk(const FInputActionValue& input)
{
	//UE_LOG(LogSagaGame, Log, TEXT("[Local][Controller] Begin Walking Strafe"));

	walkDirection.X = input.Get<FVector>().X;
	PrintVector(walkDirection);

	const auto system = USagaNetworkSubSystem::GetSubSystem(GetWorld());

	if (system->IsOfflineMode())
	{
		const auto character = GetPawn<ASagaCharacterBase>();

		character->ExecuteStraightWalk(static_cast<int>(walkDirection.Y));
		character->ExecuteStrafeWalk(static_cast<int>(walkDirection.X));
	}
	else
	{
		system->SendRpcPacket(ESagaRpcProtocol::RPC_BEG_WALK, walkDirection.X, walkDirection.Y);
	}
}

void
ASagaInGamePlayerController::MidStrafeWalk(const FInputActionValue& input)
{
	if (walkDirection.X != input.Get<FVector>().X)
	{
		BeginStrafeWalk(input);
	}
}

void
ASagaInGamePlayerController::EndStrafeWalk(const FInputActionValue& input)
{
	//UE_LOG(LogSagaGame, Log, TEXT("[Local][Controller] End Walking Strafe"));

	walkDirection.X = input.Get<FVector>().X;
	PrintVector(walkDirection);

	const auto system = USagaNetworkSubSystem::GetSubSystem(GetWorld());

	if (system->IsOfflineMode())
	{
		const auto character = GetPawn<ASagaCharacterBase>();

		character->ExecuteStraightWalk(static_cast<int>(walkDirection.Y));
		character->ExecuteStrafeWalk(static_cast<int>(walkDirection.X));
	}
	else
	{
		system->SendRpcPacket(ESagaRpcProtocol::RPC_END_WALK, walkDirection.X, walkDirection.Y);
	}
}

void
ASagaInGamePlayerController::BeginRun()
{
	//UE_LOG(LogSagaGame, Log, TEXT("[Local][Controller] Begin Running"));

	const auto system = USagaNetworkSubSystem::GetSubSystem(GetWorld());

	if (system->IsOfflineMode())
	{
		const auto character = GetPawn<ASagaCharacterBase>();

		character->ExecuteRun();
	}
	else
	{
		system->SendRpcPacket(ESagaRpcProtocol::RPC_BEG_RUN);
	}
}

void
ASagaInGamePlayerController::EndRun()
{
	//UE_LOG(LogSagaGame, Log, TEXT("[Local][Controller] End Running"));

	const auto system = USagaNetworkSubSystem::GetSubSystem(GetWorld());

	if (system->IsOfflineMode())
	{
		const auto character = GetPawn<ASagaCharacterBase>();

		character->TerminateRun();
	}
	else
	{
		system->SendRpcPacket(ESagaRpcProtocol::RPC_END_RUN);
	}
}

void
ASagaInGamePlayerController::BeginJump()
{
	//UE_LOG(LogSagaGame, Log, TEXT("[Local][Controller] Begin Jumping"));

	const auto system = USagaNetworkSubSystem::GetSubSystem(GetWorld());

	if (system->IsOfflineMode())
	{
		const auto character = GetPawn<ASagaCharacterBase>();

		character->ExecuteJump();
	}
	else
	{
		system->SendRpcPacket(ESagaRpcProtocol::RPC_BEG_JUMP);
	}
}

void
ASagaInGamePlayerController::EndJump()
{
	//UE_LOG(LogSagaGame, Log, TEXT("[Local][Controller] End Jumping"));

	const auto system = USagaNetworkSubSystem::GetSubSystem(GetWorld());

	if (system->IsOfflineMode())
	{
		const auto pawn = GetPawn<ASagaCharacterBase>();

		pawn->TerminateJump();
	}
	else
	{
		system->SendRpcPacket(ESagaRpcProtocol::RPC_END_JUMP);
	}
}

void
ASagaInGamePlayerController::BeginRotate(const FInputActionValue& input)
{
	const FVector InputValue = input.Get<FVector>();

	const auto pawn = GetPawn<ASagaCharacterBase>();
	if (IsValid(pawn))
	{
		AddYawInput(InputValue.X);

		pawn->ExecuteRotate(InputValue.Y);
	}
}

void
ASagaInGamePlayerController::BeginAttack(const FInputActionValue& input)
{
	UE_LOG(LogSagaGame, Log, TEXT("[Local][Controller] Begin Attack"));

	if (not isAttacking)
	{
		const auto system = USagaNetworkSubSystem::GetSubSystem(GetWorld());
		const auto pawn = GetPawn<ASagaCharacterBase>();

		if (pawn->IsAlive())
		{
			if (system->IsOfflineMode())
			{
				pawn->ExecuteAttack();
			}
			else
			{
				system->SendRpcPacket(ESagaRpcProtocol::RPC_BEG_ATTACK_0);
			}
		}

		isAttacking = true;
	}
}

void
ASagaInGamePlayerController::EndAttack()
{
	UE_LOG(LogSagaGame, Log, TEXT("[Local][Controller] End Attack"));

	if (isAttacking)
	{
		auto character = GetPawn<ASagaCharacterBase>();

		const auto system = USagaNetworkSubSystem::GetSubSystem(GetWorld());

		if (system->IsOfflineMode())
		{
		}
		else
		{
			system->SendRpcPacket(ESagaRpcProtocol::RPC_END_ATTACK_0);
		}

		isAttacking = false;
	}
}

void
ASagaInGamePlayerController::BeginRide()
{
	UE_LOG(LogSagaGame, Log, TEXT("[Local][Controller] Begin Ride"));

	const auto system = USagaNetworkSubSystem::GetSubSystem(GetWorld());

	if (system->IsOfflineMode())
	{
		auto character = GetPawn<ASagaCharacterBase>();


	}
	else
	{
		system->SendRpcPacket(ESagaRpcProtocol::RPC_BEG_RIDE, 0);
	}
}

void
ASagaInGamePlayerController::EndRide()
{
	const auto system = USagaNetworkSubSystem::GetSubSystem(GetWorld());

	if (system->IsOfflineMode())
	{
		const auto character = GetPawn<ASagaCharacterBase>();


	}
	else
	{
		system->SendRpcPacket(ESagaRpcProtocol::RPC_END_RIDE, 0);
	}
}
