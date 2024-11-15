#include "SagaInGameMode.h"
#include <Engine/World.h>
#include <Engine/DamageEvents.h>
#include <UObject/Object.h>
#include <Templates/Casts.h>

#include "PlayerControllers/SagaInGamePlayerController.h"
#include "Character/SagaCharacterBase.h"
#include "Character/SagaPlayableCharacter.h"
#include "Character/SagaGummyBearPlayer.h"
#include "Obstacles/MapObstacle1.h"
#include "Item/SagaItemTypes.h"
#include "Item/SagaItemBox.h"
#include "Interface/SagaCharacterItemInterface.h"
#include "SagaGameSubSystem.h"

#include "Saga/Network/SagaRpcProtocol.h"
#include "Saga/Network/SagaVirtualUser.h"
#include "Saga/Network/SagaNetworkSubSystem.h"

void
ASagaInGameMode::OnRpc(ESagaRpcProtocol cat, int32 id, int64 arg0, int32 arg1)
{
	const auto net = USagaNetworkSubSystem::GetSubSystem(GetWorld());

	bool is_remote{};

	ASagaCharacterBase* character = net->GetCharacterHandle(id);

	if (id == net->GetLocalUserId()) // 로컬 클라이언트
	{
#pragma region RPC from Local Client
		//UE_LOG(LogSagaGame, Log, TEXT("[RPC] This is my rpc message."));
#pragma endregion
	}
	else // 원격 클라이언트
	{
#pragma region RPC from Remote Client
		//UE_LOG(LogSagaGame, Log, TEXT("[RPC] This is user %d's rpc message."), id);
		is_remote = true;
#pragma endregion
	}

	switch (cat)
	{
	case ESagaRpcProtocol::RPC_UNKNOWN:
	{
#if WITH_EDITOR

		UE_LOG(LogSagaGame, Error, TEXT("[RPC] Cannot execute a rpc script for user %d."), id);
#endif
	}
	break;

	case ESagaRpcProtocol::RPC_BEG_WALK:
	{
		if (not IsValid(character))
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Warning, TEXT("[RPC_BEG_WALK] by user %d, has no character."), id);
#endif
			break;
		}

		if (is_remote)
		{
			//UE_LOG(LogSagaGame, Log, TEXT("[RPC][Remote] Begin Walking"));
		}
		else
		{
			//UE_LOG(LogSagaGame, Log, TEXT("[RPC] Begin Walking"));
		}

		const auto xdir = static_cast<int>(arg0);
		if (0 != xdir)
		{
			character->ExecuteStrafeWalk(xdir);
		}

		const auto ydir = static_cast<int>(arg1);
		if (0 != ydir)
		{
			character->ExecuteStraightWalk(ydir);
		}
	}
	break;

	case ESagaRpcProtocol::RPC_END_WALK:
	{
		if (not IsValid(character))
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Warning, TEXT("[RPC_END_WALK] by user %d, no character."), id);
#endif

			break;
		}

		if (is_remote)
		{
			//UE_LOG(LogSagaGame, Log, TEXT("[RPC][Remote] End Walking"));
		}
		else
		{
			//UE_LOG(LogSagaGame, Log, TEXT("[RPC] End Walking"));
		}

		const auto xdir = static_cast<int>(arg0);
		if (0 == xdir)
		{
			character->ExecuteStrafeWalk(xdir);
		}

		const auto ydir = static_cast<int>(arg1);
		if (0 == ydir)
		{
			character->ExecuteStraightWalk(ydir);
		}
	}
	break;

	case ESagaRpcProtocol::RPC_BEG_RUN:
	{
		if (not IsValid(character))
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Warning, TEXT("[RPC_BEG_RUN] by user %d, no character."), id);
#endif

			break;
		}

		character->ExecuteRun();
	}
	break;

	case ESagaRpcProtocol::RPC_END_RUN:
	{
		if (not IsValid(character))
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Warning, TEXT("[RPC_END_RUN] by user %d, no character."), id);
#endif

			break;
		}

		character->TerminateRun();
	}
	break;

	case ESagaRpcProtocol::RPC_BEG_JUMP:
	{
		if (not IsValid(character))
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Warning, TEXT("[RPC_BEG_JUMP] by user %d, no character."), id);
#endif

			break;
		}

		character->ExecuteJump();
	}
	break;

	// arg0: 수호자 정보
	// arg1: 수호자 식별자
	case ESagaRpcProtocol::RPC_BEG_RIDE:
	{
		if (not IsValid(character))
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Error, TEXT("[RPC_BEG_RIDE] by user %d, has no character."), id);
#endif

			break;
		}

		const auto human = Cast<ASagaPlayableCharacter>(character);

		if (not IsValid(human))
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Error, TEXT("[RPC_BEG_RIDE] User %d's character is not a human."), id);
#endif

			break;
		}

		const auto guardian_info = arg0;
		const auto guardian_id = arg1;

		const auto guardian = FindBear(guardian_id);

		if (IsValid(guardian))
		{
			if (not guardian->IsAlive())
			{
#if WITH_EDITOR

				UE_LOG(LogSagaGame, Warning, TEXT("[RPC_BEG_RIDE] The guardian with id %d is dead."), guardian_id);
#endif
				break;
			}

#if WITH_EDITOR

			const auto guardian_name = guardian->GetName();

			UE_LOG(LogSagaGame, Warning, TEXT("[RPC_BEG_RIDE] User %d would ride a guardian '%s' with id %d."), id, *guardian_name, guardian_id);
#endif

			net->SetCharacterHandle(id, guardian);

			human->ExecuteGuardianAction(guardian);
			guardian->ExecuteGuardianAction(human);

			if (is_remote)
			{
#if WITH_EDITOR

				UE_LOG(LogSagaGame, Log, TEXT("[RPC][Remote] User %d rides guardian %d"), id, guardian_id);
#endif
			}
			else // IF (is_remote)
			{
#if WITH_EDITOR

				UE_LOG(LogSagaGame, Log, TEXT("[RPC][Local] User %d rides guardian %d"), id, guardian_id);
#endif

				storedLocalPlayerController->Possess(guardian);
			} // IF NOT (is_remote)
		}
		else
		{
			UE_LOG(LogSagaGame, Warning, TEXT("[RPC_BEG_RIDE] There is no guardian with id %d."), guardian_id);
		}
	}
	break;

	// 수호자 하차
	// arg0: 수호자 정보
	// arg1: 수호자 식별자
	case ESagaRpcProtocol::RPC_END_RIDE:
	{
		const auto guardian = FindBear(arg1);

		if (not IsValid(guardian))
		{
			UE_LOG(LogSagaGame, Error, TEXT("[RPC_END_RIDE] There is no guardian with id '%d'."), arg1);

			break;
		}

		if (guardian->HasValidOwnerId())
		{
			ASagaCharacterBase* human = guardian->ownerData.GetCharacterHandle();

			if (IsValid(human))
			{
				net->SetCharacterHandle(id, human);

				human->TerminateGuardianAction();
				guardian->TerminateGuardianAction();

				const auto user_id = human->GetUserId();

				if (user_id == net->GetLocalUserId())
				{
#if WITH_EDITOR

					UE_LOG(LogSagaGame, Log, TEXT("[RPC_END_RIDE] Local user %d unrides from the guardian %d"), user_id, guardian->GetBearId());
#endif

					storedLocalPlayerController->Possess(human);
				}
				else
				{
#if WITH_EDITOR

					UE_LOG(LogSagaGame, Log, TEXT("[RPC_END_RIDE] User %d unrides from the guardian %d"), user_id, guardian->GetBearId());
#endif
				}
			}
			else
			{
#if WITH_EDITOR

				UE_LOG(LogSagaGame, Error, TEXT("[RPC_END_RIDE] User %d tried to unride from the guardian %d, but the guardian has an invalid character handle."), id, guardian->GetBearId());
#endif
			}
		}
		else
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Warning, TEXT("[RPC_END_RIDE] User %d tried to unride from the guardian %d, but the guardian has no user data."), id, guardian->GetBearId());
#endif
		}
	}
	break;

	case ESagaRpcProtocol::RPC_POSITION:
	{
		float x{};
		float y{};
		float z{};

		std::memcpy(&x, &arg0, 4);
		std::memcpy(&y, reinterpret_cast<char*>(&arg0) + 4, 4);
		std::memcpy(&z, &arg1, 4);

		if (not IsValid(character))
		{
			UE_LOG(LogSagaGame, Warning, TEXT("[RPC_POSITION] Cannot find a character of user %d'."), id);

			net->StorePosition(id, x, y, z);
		}
		else if (is_remote)
		{
			character->SetActorLocation(FVector{ x, y, z });
		}
	}
	break;

	case ESagaRpcProtocol::RPC_ROTATION:
	{
		if (not IsValid(character))
		{
			UE_LOG(LogSagaGame, Warning, TEXT("[RPC_ROTATION] by user %d, no character."), id);

			break;
		}

		if (is_remote and character->IsAlive())
		{
			float p{};
			float y{};
			float r{};

			std::memcpy(&p, &arg0, 4);
			std::memcpy(&y, reinterpret_cast<char*>(&arg0) + 4, 4);
			std::memcpy(&r, &arg1, 4);

			//UE_LOG(LogSagaGame, Log, TEXT("[RPC] Rotating remote player %d by (%f,%f,%f)."), id, p, y, r);
			character->SetActorRotation(FRotator{ p, y, r });
		}
	}
	break;

	case ESagaRpcProtocol::RPC_BEG_ATTACK_0:
	{
		if (not IsValid(character))
		{
			UE_LOG(LogSagaGame, Error, TEXT("[RPC_BEG_ATTACK_0] by user %d, has no character."), id);

			break;
		}
		else if (not character->IsAlive())
		{
			UE_LOG(LogSagaGame, Warning, TEXT("[RPC_BEG_ATTACK_0] by user %d - The character is dead."), id);

			break;
		}

		character->ExecuteAttackAnimation();
	}
	break;

	case ESagaRpcProtocol::RPC_END_ATTACK_0:
	{
		if (not IsValid(character))
		{
			UE_LOG(LogSagaGame, Error, TEXT("[RPC_END_ATTACK_0] by user %d, has no character."), id);
			break;
		}

		character->TerminateAttack();
	}
	break;

	case ESagaRpcProtocol::RPC_SKILL_0:
	{
		if (not IsValid(character))
		{
			UE_LOG(LogSagaGame, Error, TEXT("[RPC_SKILL_0] by user %d, has no character."), id);

			break;
		}
		else if (not character->IsAlive())
		{
			UE_LOG(LogSagaGame, Warning, TEXT("[RPC_SKILL_0] by user %d - The character is dead."), id);

			break;
		}

		if (const auto human = Cast<ASagaPlayableCharacter>(character))
		{
			UE_LOG(LogSagaGame, Log, TEXT("[RPC_SKILL_0] by user '%d'."), id);

			switch (human->GetWeapon())
			{
			case EPlayerWeapon::LightSabor:
			{
				human->UseSkill(0);
			}
			break;

			case EPlayerWeapon::WaterGun:
			{
				human->UseSkill(1);
			}
			break;

			case EPlayerWeapon::Hammer:
			{
				human->UseSkill(2);
			}
			break;

			default:
			{}
			break;
			}
		}
	}
	break;

	case ESagaRpcProtocol::RPC_SKILL_1:
	{
		if (not IsValid(character))
		{
			UE_LOG(LogSagaGame, Error, TEXT("[RPC_SKILL_1] by user %d, has no character."), id);

			break;
		}
		else if (not character->IsAlive())
		{
			UE_LOG(LogSagaGame, Warning, TEXT("[RPC_SKILL_1] by user %d - The character is dead."), id);

			break;
		}

		if (const auto human = Cast<ASagaPlayableCharacter>(character))
		{
			UE_LOG(LogSagaGame, Log, TEXT("[RPC_SKILL_1] by user '%d'."), id);

			switch (human->GetWeapon())
			{
			case EPlayerWeapon::LightSabor:
			{
				human->UseSkill(3);
			}
			break;

			case EPlayerWeapon::WaterGun:
			{
				human->UseSkill(4);
			}
			break;

			case EPlayerWeapon::Hammer:
			{
				human->UseSkill(5);
			}
			break;

			default:
			{}
			break;
			}
		}
	}
	break;

	case ESagaRpcProtocol::RPC_SKILL_2:
	{
		if (not IsValid(character))
		{
			UE_LOG(LogSagaGame, Error, TEXT("[RPC_SKILL_2] by user %d, has no character."), id);

			break;
		}
		else if (not character->IsAlive())
		{
			UE_LOG(LogSagaGame, Warning, TEXT("[RPC_SKILL_2] by user %d - The character is dead."), id);

			break;
		}

		if (const auto human = Cast<ASagaPlayableCharacter>(character))
		{
			UE_LOG(LogSagaGame, Log, TEXT("[RPC_SKILL_2] by user '%d'."), id);

			switch (human->GetWeapon())
			{
			case EPlayerWeapon::LightSabor:
			{
				human->UseSkill(6);
			}
			break;

			case EPlayerWeapon::WaterGun:
			{
				human->UseSkill(7);
			}
			break;

			case EPlayerWeapon::Hammer:
			{
				human->UseSkill(8);
			}
			break;

			default:
			{}
			break;
			}
		}
	}
	break;

	case ESagaRpcProtocol::RPC_SKILL_3:
	{
		if (not IsValid(character))
		{
			UE_LOG(LogSagaGame, Error, TEXT("[RPC_SKILL_3] by user %d, has no character."), id);

			break;
		}
		else if (not character->IsAlive())
		{
			UE_LOG(LogSagaGame, Warning, TEXT("[RPC_SKILL_3] by user %d - The character is dead."), id);

			break;
		}

		if (const auto human = Cast<ASagaPlayableCharacter>(character))
		{
			UE_LOG(LogSagaGame, Log, TEXT("[RPC_SKILL_3] by user '%d'."), id);

			switch (human->GetWeapon())
			{
			case EPlayerWeapon::LightSabor:
			{
				human->UseSkill(9);
			}
			break;

			case EPlayerWeapon::WaterGun:
			{
				human->UseSkill(10);
			}
			break;

			case EPlayerWeapon::Hammer:
			{
				human->UseSkill(11);
			}
			break;

			default:
			{}
			break;
			}
		}
	}
	break;

	case ESagaRpcProtocol::RPC_DESTROY_ITEM_BOX:
	{
#if WITH_EDITOR

		UE_LOG(LogSagaGame, Log, TEXT("[RPC_DESTROY_ITEM_BOX] target item id: %d"), arg1);
#endif

		/// **
		net->SendRpcPacket(ESagaRpcProtocol::RPC_GET_SCORE);

		for (auto& item_spawner : everyItemSpawnEntities)
		{
			if (IsValid(item_spawner.Get()) and item_spawner->GetID() == arg1)
			{
				//const auto box = item_spawner->SpawnItemBox();
				//ensure(IsValid(box));

				//everyItemBoxes.Add(box);

#if WITH_EDITOR

				//const auto name = box->GetName();

				//UE_LOG(LogSagaGame, Log, TEXT("[RPC_DESTROY_ITEM_BOX] item %s is spawned."), *name);
#endif

				//item_spawner->Destroy();
				//everyItemSpawnEntities.RemoveSwap(item_spawner);
				break;
			}
		}
	}
	break;

	case ESagaRpcProtocol::RPC_GRAB_ITEM:
	{
		UE_LOG(LogSagaGame, Log, TEXT("[RPC_GRAB_ITEM] item id: %d"), arg1);

		for (auto& item_spawner : everyItemSpawnEntities)
		{
			if (IsValid(item_spawner.Get()) and item_spawner->GetID() == arg1)
			{
#if WITH_EDITOR

				const auto name = item_spawner->GetName();

				UE_LOG(LogSagaGame, Log, TEXT("[RPC_GRAB_ITEM] item spawner %s is destroyed."), *name);
#endif

				item_spawner->Destroy();
				break;
			}
		}

		for (auto& item_box : everyItemBoxes)
		{
			if (IsValid(item_box.Get()) and item_box->GetItemId() == arg1)
			{
#if WITH_EDITOR

				const auto name = item_box->GetName();

				UE_LOG(LogSagaGame, Log, TEXT("[RPC_GRAB_ITEM] item %d (%s) is destroyed."), arg1, *name);
#endif

				item_box->Destroy();
				//everyItemBoxes.RemoveSwap(item_box);
				break;
			}
		}

		if (not is_remote)
		{
			ISagaCharacterItemInterface* item_inf = Cast<ISagaCharacterItemInterface>(character);

			if (item_inf)
			{
				const auto rng_item = FMath::RandRange(0, 2);

				UE_LOG(LogSagaGame, Log, TEXT("[RPC_GRAB_ITEM] User %d is getting item."), rng_item);

				item_inf->TakeItem(static_cast<ESagaItemTypes>(rng_item));
			}
		}
	}
	break;

	case ESagaRpcProtocol::RPC_USE_ITEM_0:
	{
		UE_LOG(LogSagaGame, Log, TEXT("[RPC_USE_ITEM_0] item id: %d"), arg1);

		switch (arg1)
		{
		case 0:
		{
			UE_LOG(LogSagaGame, Log, TEXT("[RPC_USE_ITEM_0] Energy Drink"));

			character->ExecuteUseItem(ESagaItemTypes::Drink);
		}
		break;

		case 1:
		{
			UE_LOG(LogSagaGame, Log, TEXT("[RPC_USE_ITEM_0] Gumball"));

			character->ExecuteUseItem(ESagaItemTypes::Gum);
		}
		break;

		case 2:
		{
			UE_LOG(LogSagaGame, Log, TEXT("[RPC_USE_ITEM_0] Smoke Bomb"));

			character->ExecuteUseItem(ESagaItemTypes::SmokeBomb);
		}
		break;

		default:
		{
			UE_LOG(LogSagaGame, Error, TEXT("[USagaInventoryWidget] Invalid item id"));
		}
		break;
		}
	}
	break;

	case ESagaRpcProtocol::RPC_MAIN_WEAPON:
	{
		const auto new_weapon = static_cast<EPlayerWeapon>(arg0);
		net->SetWeapon(id, new_weapon);

		const auto name = UEnum::GetValueAsString(new_weapon);

		UE_LOG(LogSagaGame, Warning, TEXT("[RPC] %d's weapon is %s"), id, *name);
	}
	break;

	case ESagaRpcProtocol::RPC_DMG_PLYER:
	{
		if (IsValid(character))
		{
			UE_LOG(LogSagaGame, Log, TEXT("[RPC_DMG_PLYER] by user %d."), id);
		}
		else
		{
			UE_LOG(LogSagaGame, Warning, TEXT("[RPC_DMG_PLYER] by user %d, has no character."), id);

			break;
		}

		float dmg{};
		memcpy(&dmg, reinterpret_cast<const char*>(&arg0), 4);

		UE_LOG(LogSagaGame, Log, TEXT("[RPC_DMG_PLYER] User %d is damaged for %f"), id, dmg);

		const int32 causer_id = arg1;

		FSagaVirtualUser other_user{};
		if (net->FindUser(causer_id, other_user))
		{
			const auto other_character = other_user.GetCharacterHandle();

			if (IsValid(other_character))
			{
				UE_LOG(LogSagaGame, Log, TEXT("[RPC_DMG_PLYER] User %d is taking damage from user %d."), id, causer_id);
			}
			else
			{
				UE_LOG(LogSagaGame, Error, TEXT("[RPC_DMG_PLYER] Causer %d does not have a character."), causer_id);
			}
		}
		else if (-2 == causer_id)
		{
			///TODO: 된다면 작은 곰에도 ID 부여해야함, 나중에 확인 필요
			UE_LOG(LogSagaGame, Log, TEXT("[RPC_DMG_PLYER] User %d is taking damage from the mini bear."), id);
		}
		else
		{
			UE_LOG(LogSagaGame, Error, TEXT("[RPC_DMG_PLYER] Causer %d does not exist."), causer_id);
		}

		/// **
		net->SendRpcPacket(ESagaRpcProtocol::RPC_GET_SCORE);

		character->ExecuteHurt(dmg);
	}
	break;

	// arg0: 수호자에 준 피해량 (4 바이트)
	// arg1: 수호자 식별자
	case ESagaRpcProtocol::RPC_DMG_GUARDIAN:
	{
		if (IsValid(character))
		{
			UE_LOG(LogSagaGame, Log, TEXT("[RPC_DMG_GUARDIAN] by user %d."), id);
		}
		else
		{
			UE_LOG(LogSagaGame, Warning, TEXT("[RPC_DMG_GUARDIAN] by user %d, has no character."), id);

			break;
		}

		const auto guardian_info = arg0;
		const auto guardian_id = arg1;

		const auto guardian = FindBear(guardian_id);

		if (IsValid(guardian))
		{
			float dmg{};
			memcpy(&dmg, reinterpret_cast<const char*>(&arg0), 4);

#if WITH_EDITOR

			UE_LOG(LogSagaGame, Log, TEXT("[RPC_DMG_GUARDIAN] Guardian %d is damaged for %f"), guardian_id, dmg);
#endif

			if (not guardian->IsAlive())
			{
#if WITH_EDITOR

				UE_LOG(LogSagaGame, Warning, TEXT("[RPC_DMG_GUARDIAN] The guardian with id %d is already dead."), guardian_id);
#endif
			}
			else
			{
				guardian->ExecuteHurt(dmg);
			}

			// NOTICE: else문으로 연결하면 안됨
			if (not guardian->IsAlive())
			{
#if WITH_EDITOR

				UE_LOG(LogSagaGame, Warning, TEXT("[RPC_DMG_GUARDIAN] The guardian with id %d is dead."), guardian_id);

				const auto guardian_name = guardian->GetName();

				UE_LOG(LogSagaGame, Warning, TEXT("[RPC_DMG_GUARDIAN] User %d would unride from the guardian '%s' with id %d."), id, *guardian_name, guardian_id);
#endif

				// 사망 시에는 하차 처리를 함.
				ASagaCharacterBase* human = guardian->ownerData.GetCharacterHandle();

				if (IsValid(human))
				{
					net->SetCharacterHandle(id, human);

					human->TerminateGuardianAction();
					guardian->TerminateGuardianAction();

					if (is_remote)
					{
#if WITH_EDITOR

						UE_LOG(LogSagaGame, Log, TEXT("[RPC_DMG_GUARDIAN][Remote] User %d would unride from the guardian %d"), id, guardian_id);
#endif
					}
					else // IF NOT (is_remote)
					{
#if WITH_EDITOR

						UE_LOG(LogSagaGame, Log, TEXT("[RPC_DMG_GUARDIAN][Local] User %d would unride from the guardian %d"), id, guardian_id);
#endif

						storedLocalPlayerController->Possess(human);
					} // IF NOT (is_remote)
				}
			}
		}
		else
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Error, TEXT("[RPC_BEG_RIDE] There is no guardian with id %d."), guardian_id);
#endif
		}

		net->SendRpcPacket(ESagaRpcProtocol::RPC_GET_SCORE);
	}
	break;

	// arg0: 파괴된 부위
	// arg1: 수호자 식별자
	case ESagaRpcProtocol::RPC_DMG_GUARDIANS_PART:
	{
		const auto guardian_part_id = static_cast<int32>(arg0);
		const auto guardian_id = arg1;

		const auto guardian = FindBear(guardian_id);

		if (IsValid(guardian))
		{
#if WITH_EDITOR

			const auto guardian_name = guardian->GetName();
			UE_LOG(LogSagaGame, Log, TEXT("[RPC_DMG_GUARDIANS_PART] By user %d to part %d of guardian %d."), id, guardian_part_id, guardian_id);
#endif

			if (not guardian->IsAlive())
			{
#if WITH_EDITOR

				//UE_LOG(LogSagaGame, Log, TEXT("[RPC_DMG_GUARDIANS_PART] The guardian with id %d is already dead."), guardian_id);
#endif
			}

			if (guardian->IsPartDestructed(guardian_part_id))
			{
#if WITH_EDITOR

				UE_LOG(LogSagaGame, Warning, TEXT("[RPC_DMG_GUARDIANS_PART] '%s''s part %d is already destructed."), *guardian_name, guardian_part_id);
#endif
			}
			else
			{
				if (guardian->GiveDamageToPart(guardian_part_id))
				{
#if WITH_EDITOR

					UE_LOG(LogSagaGame, Log, TEXT("[RPC_DMG_GUARDIANS_PART] Destructing '%s''s part %d."), *guardian_name, guardian_part_id);
#endif

					const auto part_actor = guardian->ExecutePartDestruction(guardian_part_id);

					if (part_actor)
					{
						net->SendRpcPacket(ESagaRpcProtocol::RPC_MORPH_GUARDIANS_PART, guardian_part_id, guardian_id);
						//guardian->ExecuteMorphingPart(part_actor, guardian_part_id);
					}
				}
			}
		}
		else
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Error, TEXT("[RPC_DMG_GUARDIANS_PART] Cannot found the guardian '%d'. (User: %d, Part: %d)"), guardian_id, id, guardian_part_id);
#endif
		}
	}
	break;

	case ESagaRpcProtocol::RPC_MORPH_GUARDIANS_PART:
	{
		const auto guardian_part_id = static_cast<int32>(arg0);
		const auto guardian_id = arg1;

		const auto guardian = FindBear(guardian_id);

		if (IsValid(guardian))
		{
			if (not guardian->IsPartDestructed(guardian_part_id))
			{
#if WITH_EDITOR

				const auto guardian_name = guardian->GetName();
				UE_LOG(LogSagaGame, Log, TEXT("[RPC_MORPH_GUARDIANS_PART] Guardian '%s'(%d)'s part %d is just destroyed and morphing."), *guardian_name, guardian_id, guardian_part_id);
#endif

				guardian->GiveDamageToPart(guardian_part_id);
				const auto part_actor = guardian->ExecutePartDestruction(guardian_part_id);

				if (part_actor)
				{
					guardian->ExecuteMorphingPart(part_actor, guardian_part_id);
				}
			}
			else
			{
#if WITH_EDITOR

				const auto guardian_name = guardian->GetName();
				UE_LOG(LogSagaGame, Log, TEXT("[RPC_MORPH_GUARDIANS_PART] Guardian '%s'(%d)'s part %d is morphing."), *guardian_name, guardian_id, guardian_part_id);
#endif

				guardian->ExecuteMorphingPartAt(guardian_part_id);
			}
		}
		else
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Error, TEXT("[RPC_MORPH_GUARDIANS_PART] There is no guardian with ab id '%d'."), guardian_id);
#endif
		}
	}
	break;

	case ESagaRpcProtocol::RPC_ACTIVE_GUARDIANS_PART:
	{

	}
	break;

	case ESagaRpcProtocol::RPC_DMG_BASE:
	{
		if (is_remote)
		{

		}
	}
	break;

	case ESagaRpcProtocol::RPC_DEAD:
	{
		if (IsValid(character))
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Log, TEXT("[RPC_DEAD] User %d is dead."), id);
#endif
		}
		else
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Error, TEXT("[RPC_DEAD] User %d is dead, has no character."), id);
#endif

			break;
		}

		/// **
		net->SendRpcPacket(ESagaRpcProtocol::RPC_GET_SCORE);

		character->ExecuteDeath();
	}
	break;

	case ESagaRpcProtocol::RPC_RESPAWN:
	{
		/// **
		net->SendRpcPacket(ESagaRpcProtocol::RPC_GET_SCORE);

		if (IsValid(character))
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Log, TEXT("[RPC_RESPAWN] User %d is respawning..."), id);
#endif

			character->ExecuteRespawn();
		}
		else
		{
			ESagaPlayerTeam team{};

			if (net->GetTeam(id, team))
			{
				if (is_remote)
				{
#if WITH_EDITOR

					UE_LOG(LogSagaGame, Warning, TEXT("[RPC_RESPAWN][Remote] User %d is respawning, no character."), id);
#endif
				}
				else
				{
#if WITH_EDITOR

					UE_LOG(LogSagaGame, Warning, TEXT("[RPC_RESPAWN][Local] User %d is respawning, no character."), id);
#endif
				}
			}
			else
			{
#if WITH_EDITOR

				UE_LOG(LogSagaGame, Error, TEXT("[RPC_RESPAWN] User %d could not find its team!"), id);
#endif

				return;
			}

			break;
		}
	}
	break;

	case ESagaRpcProtocol::RPC_RESPAWN_TIMER:
	{
#if WITH_EDITOR

		UE_LOG(LogSagaGame, Log, TEXT("[RPC_RESPAWN_TIMER] Time: %d"), arg0);
#endif

		/// **
		net->SendRpcPacket(ESagaRpcProtocol::RPC_GET_SCORE);
	}
	break;

	case ESagaRpcProtocol::RPC_GET_SCORE:
	{
#if WITH_EDITOR

		//UE_LOG(LogSagaGame, Log, TEXT("[RPC_GET_SCORE] red: %d, blu: %d"), arg0, arg1);
#endif

		const auto world = GetWorld();
		const auto sys = USagaGameSubsystem::GetSubSystem(world);

		sys->SetScore(ESagaPlayerTeam::Red, arg0);
		sys->SetScore(ESagaPlayerTeam::Blue, arg1);
	}
	break;

	// arg0 : score of red team (4 bytes) | score of blu team (4 bytes)
	// arg1 : winner id (0 - error, 1 - red, 2 - blu, 3 - draw)
	case ESagaRpcProtocol::RPC_GAME_END:
	{
		// Determine whether who is win
		int32 red_score{}, blu_score{}, winner{ arg1 };

		memcpy(&red_score, reinterpret_cast<const char*>(&arg0), 4);
		memcpy(&blu_score, reinterpret_cast<const char*>(&arg0) + 4, 4);

#if WITH_EDITOR

		UE_LOG(LogSagaGame, Log, TEXT("[RPC_GAME_END] Winner: %d"), winner);
#endif

		if (winner == 1)
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Log, TEXT("[RPC_GAME_END] Red team has won the game!"));
#endif
		}
		else if (winner == 2)
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Log, TEXT("[RPC_GAME_END] Blue team has won the game!"));
#endif
		}
		else if (winner == 3)
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Log, TEXT("[RPC_GAME_END] Both teams has drawn to the game!"));
#endif
		}
		else
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Error, TEXT("[RPC_GAME_END] User %d received unknown winner!"), id);
#endif
		}

		const auto world = GetWorld();
		const auto sys = USagaGameSubsystem::GetSubSystem(world);

		sys->SetScore(ESagaPlayerTeam::Red, red_score);
		sys->SetScore(ESagaPlayerTeam::Blue, blu_score);

		if (sys->TrySetWinner(FMath::Clamp(winner, 1, 3)))
		{
			UGameplayStatics::OpenLevel(this, TEXT("GameEndLevel"));
		}
		else
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Warning, TEXT("[RPC_GAME_END] User %d already have set the winner!"), id);
#endif
		}
	}
	break;

	// arg0 : score of red team (4 bytes) | score of blu team (4 bytes)
	// arg1 : winner id (0 - error, 1 - red, 2 - blu)
	case ESagaRpcProtocol::RPC_DESTROY_CORE:
	{
		// Determine whether who is win
		int32 red_score{}, blu_score{}, winner{ arg1 };

		memcpy(&red_score, reinterpret_cast<const char*>(&arg0), 4);
		memcpy(&blu_score, reinterpret_cast<const char*>(&arg0) + 4, 4);

#if WITH_EDITOR

		UE_LOG(LogSagaGame, Log, TEXT("[RPC_DESTROY_CORE] Red score: %d, Blue score: %d, Winner: %d"), red_score, blu_score, winner);
#endif

		const auto world = GetWorld();
		const auto sys = USagaGameSubsystem::GetSubSystem(world);

		if (net->GetLocalUserTeam() == ESagaPlayerTeam::Red)
		{
		}
		else if (net->GetLocalUserTeam() == ESagaPlayerTeam::Blue)
		{

		}
		else
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Error, TEXT("[RPC_DESTROY_CORE] User %d is in the unknown team!"), id);
#endif
		}

		sys->SetScore(ESagaPlayerTeam::Red, red_score);
		sys->SetScore(ESagaPlayerTeam::Blue, blu_score);

		if (winner == 1)
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Log, TEXT("[RPC_DESTROY_CORE] Red team has won the game!"));
#endif

			sys->SetWhoWonByPinata(1);
			auto& timer = GetWorldTimerManager();
			timer.SetTimer(LevelChangeTimerHandle, this, &ASagaInGameMode::ChangeToEndLevel, 4.0f, false);
		}
		else if (winner == 2)
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Log, TEXT("[RPC_DESTROY_CORE] Blue team has won the game!"));
#endif

			sys->SetWhoWonByPinata(0);
			auto& timer = GetWorldTimerManager();
			timer.SetTimer(LevelChangeTimerHandle, this, &ASagaInGameMode::ChangeToEndLevel, 4.0f, false);
		}
		else
		{
#if WITH_EDITOR

			UE_LOG(LogSagaGame, Error, TEXT("[RPC_DESTROY_CORE] User %d received unknown winner!"), id);
#endif
		}
	}
	break;

	default:
		break;
	}
}
