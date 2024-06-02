#include "SagaInGameMode.h"
#include <Logging/LogMacros.h>
#include <Engine/World.h>
#include <EngineUtils.h>
#include <UObject/Object.h>
#include <GameFramework/Actor.h>

#include "Character/SagaInGamePlayerController.h"
#include "Character/SagaCharacterPlayer.h"

#include "Saga/Network/SagaRpcProtocol.h"
#include "Saga/Network/SagaVirtualUser.h"
#include "Saga/Network/SagaNetworkSubSystem.h"

void
ASagaInGameMode::OnLeftRoom(int32 user_id)
{
	const auto world = GetWorld();
	const auto system = USagaNetworkSubSystem::GetSubSystem(world);

	FSagaVirtualUser user{};

	if (system->FindUser(user_id, user))
	{
		const auto character = user.GetCharacterHandle();

		if (IsValid(character))
		{
			system->SetCharacterHandle(user_id, nullptr);
			character->Destroy();
		}
	}
	else
	{
		UE_LOG(LogSagaGame, Error, TEXT("[OnLeftRoom] Could not find the user %d."), user_id);

		// 어쨌든 찾아서 삭제함
		for (TActorIterator<ASagaCharacterPlayer> it{ world }; it; ++it)
		{
			const auto character = *it;

			if (IsValid(character) && character->GetUserId() == user_id)
			{
				character->Destroy();
			}
		}
	}
}

void
ASagaInGameMode::OnGameStarted()
{
	const auto world = GetWorld();
	const auto system = USagaNetworkSubSystem::GetSubSystem(world);

	if (not system->IsOfflineMode())
	{
		UE_LOG(LogSagaGame, Log, TEXT("[OnGameStarted] Game is started."));
	}
	else
	{
		UE_LOG(LogSagaGame, Log, TEXT("[OnGameStarted] Game is started. (Offline Mode)"));
	}

	world->bMatchStarted = true;
}

void ASagaInGameMode::OnCreatingCharacter(int32 user_id, EUserTeam team, EPlayerWeapon weapon)
{
	const auto world = GetWorld();
	const auto system = USagaNetworkSubSystem::GetSubSystem(world);
	const auto controller = world->GetFirstPlayerController<ASagaInGamePlayerController>();

	if (not system->IsOfflineMode())
	{
		if (not system->IsConnected())
		{
			UE_LOG(LogSagaGame, Error, TEXT("[OnCreatingCharacter] Client is not connected."));
			return;
		}

		const auto local_id = system->GetLocalUserId();
		if (-1 == local_id)
		{
			UE_LOG(LogSagaGame, Error, TEXT("[OnCreatingCharacter] Local user could not create a playable character."));
			return;
		}

		ASagaCharacterPlayer* character;

		if (local_id == user_id)
		{
			UE_LOG(LogSagaGame, Log, TEXT("[OnCreatingCharacter] Local user `%d` doesn't need to create a character."), user_id);

			// NOTICE: 여기서 로컬 캐릭터 할당
			character = controller->GetPawn<ASagaCharacterPlayer>();
			system->SetCharacterHandle(local_id, character);

			// The id was stored on LobbyLevel
			character->SetUserId(user_id);
			// The team was stored on RoomSessionLevel
			character->SetTeamColorAndCollision(team);
			// The weapon was stored on CharacterSelectLevel
			character->SetWeapon(weapon);
			character->AttachWeapon();

			// Making a spawn point
			const FVector pos = character->GetActorLocation();

			// store the initial spawn point (local)
			system->StorePosition(user_id, pos.X, pos.Y, pos.Z);

			UE_LOG(LogSagaGame, Log, TEXT("[OnCreatingCharacter] Local user `%d` created a spawn point at (%f,%f,%f)"), user_id, pos.X, pos.Y, pos.Z);

			int64 arg0{};
			int32 arg1{};
			const auto x = static_cast<float>(pos.X);
			const auto y = static_cast<float>(pos.Y);
			const auto z = static_cast<float>(pos.Z);

			memcpy(reinterpret_cast<char*>(&arg0), &x, 4);
			memcpy(reinterpret_cast<char*>(&arg0) + 4, &y, 4);
			memcpy(reinterpret_cast<char*>(&arg1), &z, 4);

			// let others know this
			system->SendRpcPacket(ESagaRpcProtocol::RPC_POSITION, arg0, arg1);
		}
		else
		{
			UE_LOG(LogSagaGame, Log, TEXT("[OnCreatingCharacter] User `%d` would create a playable character."), user_id);

			const AActor* spawner = GetSpawnerBy(team);
			if (nullptr == spawner)
			{
				spawner = world->GetWorldSettings();
			}

			FActorSpawnParameters setting{};
			setting.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			const FTransform& transform = spawner->GetTransform();

			character = world->SpawnActor<ASagaPlayableCharacter>(DefaultPawnClass, transform, setting);

			if (IsValid(character))
			{
				system->SetCharacterHandle(user_id, character);

				// The id was stored on LobbyLevel
				character->SetUserId(user_id);
				// The team was stored on RoomSessionLevel
				character->SetTeamColorAndCollision(team);
				// The weapon was stored on CharacterSelectLevel
				character->SetWeapon(weapon);
				character->AttachWeapon();

				// store the initial spawn point (remote)
				const auto pos = transform.GetLocation();
				system->StorePosition(user_id, pos.X, pos.Y, pos.Z);
			}
			else
			{
				UE_LOG(LogSagaGame, Error, TEXT("[OnCreatingCharacter] User %d could not create a character!"), user_id);
			}
		}
	}
	else
	{
		UE_LOG(LogSagaGame, Log, TEXT("[OnCreatingCharacter] Local user `%d` doesn't need to create a character. (Offline Mode)"), user_id);

		const auto character = controller->GetPawn<ASagaCharacterPlayer>();
		system->SetCharacterHandle(user_id, character);

		character->SetUserId(user_id);
		character->SetTeamColorAndCollision(team);
		character->SetWeapon(weapon);
		character->AttachWeapon();
	}
}
