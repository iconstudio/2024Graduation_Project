#include "Saga/Network/SagaNetworkEventRouter.h"

#include "Saga/Network/SagaNetworkSubSystem.h"
#include "Saga/Network/SagaPacketProcessor.h"
#include "Saga/Network/SagaGameContract.h"
#include "Saga/Network/SagaVirtualUser.h"
#include "SagaGame/Player/SagaUserTeam.h"
#include "SagaGame/Character/SagaCharacterPlayer.h"
#include "SagaGame/Character/SagaPlayableCharacter.h"

void
USagaNetworkSubSystem::RouteEvents(const std::byte* packet_buffer, EPacketProtocol protocol, int16 packet_size)
{
	switch (protocol)
	{
	case EPacketProtocol::SC_SIGNIN_SUCCESS:
	{
		int32 my_id{};
		saga::ReceiveSignInSucceedPacket(packet_buffer, my_id);

		UE_LOG(LogSagaNetwork, Log, TEXT("Local client's id is %d"), my_id);

		CallFunctionOnGameThread([this, my_id]()
			{
				SetLocalUserId(my_id);

				BroadcastOnConnected();
			}
		);
	}
	break;

	case EPacketProtocol::SC_SIGNIN_FAILURE:
	{
		int32 error{};
		saga::ReceiveSignInFailurePacket(packet_buffer, error);

		CallFunctionOnGameThread([this]()
			{
				SetLocalUserId(-1);

				BroadcastOnFailedToConnect(ESagaConnectionContract::SignInFailed);
			}
		);
	}
	break;

	case EPacketProtocol::SC_ROOM_CREATED:
	{
		int32 room_id{};
		saga::ReceiveRoomCreatedPacket(packet_buffer, room_id);

		CallFunctionOnGameThread([this, room_id]()
			{
				SetCurrentRoomId(room_id);

				BroadcastOnRoomCreated(room_id);
			}
		);
	}
	break;

	case EPacketProtocol::SC_ROOM_CREATE_FAILED:
	{
		ERoomContract error{};
		saga::ReceiveRoomCreationFailedPacket(packet_buffer, error);

		const auto msg = std::to_wstring(error);
		UE_LOG(LogSagaNetwork, Log, TEXT("Could not create a room due to %s"), msg.data());
	}
	break;

	case EPacketProtocol::SC_ROOM_JOINED:
	{
		saga::datagrams::SerializedMember newbie{};
		int32 room_id{};
		saga::ReceiveRoomJoinedPacket(packet_buffer, newbie, room_id, GetLocalUserId());

		if (newbie.id == GetLocalUserId())
		{
			UE_LOG(LogSagaNetwork, Log, TEXT("Local client has joined to the room %d"), room_id);

			CallFunctionOnGameThread([this, room_id, newbie_id = newbie.id]()
				{
					SetCurrentRoomId(room_id);

					BroadcastOnJoinedRoom(newbie_id);
				}
			);
		}
		else
		{
			auto nickname = FString{ newbie.nickname };
			UE_LOG(LogSagaNetwork, Log, TEXT("Client %d [%s] has joined to the room %d"), newbie.id, *nickname, room_id);

			CallFunctionOnGameThread(
				[this, newbie = MoveTemp(newbie), nickname = MoveTempIfPossible(nickname)]()
				{
					AddUser(FSagaVirtualUser
						{
							newbie.id,
							MoveTempIfPossible(nickname),
							nullptr,
							nullptr,
							static_cast<EUserTeam>(newbie.team_id)
						}
					);

					BroadcastOnJoinedRoom(newbie.id);
				}
			);
		}
	}
	break;

	case EPacketProtocol::SC_ROOM_JOIN_FAILED:
	{
		ERoomContract error{};
		saga::ReceiveRoomJoinFailedPacket(packet_buffer, error);

		const auto msg = std::to_wstring(error);
		UE_LOG(LogSagaNetwork, Log, TEXT("Failed to join to a room due to %s"), msg.data());
	}
	break;

	case EPacketProtocol::SC_ROOM_LEFT:
	{
		int32 left_client_id{};
		saga::ReceiveRoomLeftPacket(packet_buffer, left_client_id);

		if (left_client_id == GetLocalUserId())
		{
			CallFunctionOnGameThread([this]()
				{
					SetCurrentRoomId(-1);
					ClearUserList();

					BroadcastOnLeftRoomBySelf();
				}
			);
		}
		else
		{
			CallFunctionOnGameThread([this, left_client_id]()
				{
					RemoveUser(left_client_id);

					BroadcastOnLeftRoom(left_client_id);
				}
			);
		}
	}
	break;

	case EPacketProtocol::SC_RESPOND_VERSION:
	{
		wchar_t version_string[16]{};

		saga::ReceiveRespondVersionPacket(packet_buffer, version_string, 16);

		CallPureFunctionOnGameThread([this, version_string]()
			{
				BroadcastOnRespondVersion(version_string);
			}
		);
	}
	break;

	case EPacketProtocol::SC_RESPOND_ROOMS:
	{
		std::vector<saga::datagrams::SerializedRoom> rooms{};

		saga::ReceiveRespondRoomsPacket(packet_buffer, rooms);

		UE_LOG(LogSagaNetwork, Log, TEXT("Number of rooms: %d"), rooms.size());

		CallFunctionOnGameThread([this, tr_rooms = std::move(rooms)]()
			{
				ClearRoomList();
				for (auto& room : tr_rooms)
				{
					AddRoom(FSagaVirtualRoom
						{
							room.id, room.title, static_cast<int>(room.members)
						});
					UE_LOG(LogSagaNetwork, Log, TEXT("Room (%d): %s (%d/4)"), room.id, room.title, room.members);
				}

				BroadcastOnUpdateRoomList(everyRooms);
			}
		);
	}
	break;

	case EPacketProtocol::SC_RESPOND_USERS:
	{
		std::vector<saga::datagrams::SerializedMember> users{};

		saga::ReceiveRespondUsersPacket(packet_buffer, users);

		UE_LOG(LogSagaNetwork, Log, TEXT("Members: %d"), users.size());

		CallFunctionOnGameThread([this, tr_users = std::move(users)]()
			{
				ClearUserList();
				for (auto& user : tr_users)
				{
					const auto team_id = user.team_id == 1 ? EUserTeam::Red : EUserTeam::Blue;

					AddUser(FSagaVirtualUser
						{
							user.id,
							user.nickname,
							nullptr,
							nullptr,
							team_id
						});

					auto str = UEnum::GetValueAsString(team_id);
					UE_LOG(LogSagaNetwork, Log, TEXT("User (%d): %s in team '%s'"), user.id, user.nickname, *str);
				}

				BroadcastOnUpdateMembers(everyUsers);
			}
		);
	}
	break;

	case EPacketProtocol::SC_SET_TEAM:
	{
		int32 client_id{};
		bool is_red_team{};

		saga::ReceiveTeamChangerPacket(packet_buffer, client_id, is_red_team);

		UE_LOG(LogSagaNetwork, Log, TEXT("Client %d's team changed to %s"), client_id, is_red_team ? TEXT("Red") : TEXT("Blue"));

		for (auto& member : everyUsers)
		{
			if (member.MyID == client_id)
			{
				member.myTeam = is_red_team ? EUserTeam::Red : EUserTeam::Blue;
			}
		}

		BroadcastOnTeamChanged(client_id, is_red_team);
	}
	break;

	case EPacketProtocol::SC_FAILED_GAME_START:
	{
		saga::SC_FailedGameStartingPacket pk{};
		auto offset = pk.Read(packet_buffer);

		const ESagaGameContract cause = static_cast<ESagaGameContract>(static_cast<uint8>(pk.errCause));
		auto str = UEnum::GetValueAsString(cause);

		UE_LOG(LogSagaNetwork, Log, TEXT("Failed to start game due to %s"), *str);

		BroadcastOnFailedToStartGame(cause);
	}
	break;

	case EPacketProtocol::SC_GAME_GETTING_READY:
	{
		//SC_ReadyForGamePacket pk{};
		//auto offset = pk.Read(packet_buffer);

		UE_LOG(LogSagaNetwork, Log, TEXT("Now start loading game..."));

		BroadcastOnGetPreparedGame();
	}
	break;

	case EPacketProtocol::SC_GAME_START:
	{
		UE_LOG(LogSagaNetwork, Log, TEXT("Now start game..."));

		BroadcastOnStartGame();
	}
	break;

	case EPacketProtocol::SC_CREATE_PLAYER:
	{
		UE_LOG(LogSagaNetwork, Log, TEXT("[SagaGame] %d Characters would be created"), everyUsers.Num());


		const auto player = GEngine->FindFirstLocalPlayerFromControllerId(0);
		if (nullptr == player)
		{
			UE_LOG(LogSagaNetwork, Error, TEXT("[SagaGame][RPC] Cannot find a handle of the local player."));
			return;
		}

		const auto world = player->GetWorld();
		if (nullptr == world)
		{
			UE_LOG(LogSagaNetwork, Error, TEXT("[SagaGame][RPC] The handle of world is null."));
			return;
		}

		auto controller = player->GetPlayerController(world);
		if (nullptr == controller)
		{
			UE_LOG(LogSagaNetwork, Error, TEXT("[SagaGame][RPC] Cannot find the local player's controller."));
			return;
		}

		// 여기서 로컬 캐릭터 할당
		auto my_pawn = controller->GetPawn();
		localPlayerCharacter = my_pawn;

		CallFunctionOnGameThread([this, my_pawn]()
			{
				auto remote_character_class = dummyPlayerClassReference.LoadSynchronous();

				for (auto& member : everyUsers)
				{
					if (member.ID() == GetLocalUserId())
					{
						UE_LOG(LogSagaNetwork, Log, TEXT("[SagaGame] Local client `%d` doesn't need to create a character."), member.ID());

						member.remoteCharacter = Cast<ASagaCharacterPlayer>(Cast<ACharacter>(my_pawn));
					}
					else
					{
						FTransform transform{};
						transform.TransformPosition({ 0, 0, 100 });

						auto character = Cast<ASagaCharacterPlayer>(CreatePlayableCharacter(remote_character_class, transform));
						if (character)
						{
							member.remoteCharacter = character;

							BroadcastOnCreatingCharacter(member.ID(), member.myTeam, member.remoteCharacter);

							UE_LOG(LogSagaNetwork, Log, TEXT("[SagaGame] User `%d` created a playable character"), member.ID());
						}
						else
						{
							UE_LOG(LogSagaNetwork, Error, TEXT("[SagaGame] User `%d` could not create a playable character"), member.ID());
						}
					}
				}
			}
		);
	}
	break;

	case EPacketProtocol::SC_REMOVE_PLAYER:
	{
		saga::SC_DestroyPlayerPacket pk{};
		auto offset = pk.Read(packet_buffer);

		UE_LOG(LogSagaNetwork, Log, TEXT("[SagaGame] A client %d is destroyed(disconnected)"), pk.clientId);

		CallFunctionOnGameThread([this, pk]()
			{
			}
		);
	}
	break;

	case EPacketProtocol::SC_MOVE_CHARACTER:
	{
		int32 client_id{};
		float x{}, y{}, z{};

		saga::ReceivePositionPacket(packet_buffer, client_id, x, y, z);

		UE_LOG(LogSagaNetwork, Log, TEXT("[SagaGame] Client id %d: pos(%f,%f,%f)"), client_id, x, y, z);

		CallFunctionOnGameThread([this, client_id, x, y, z]()
			{
				BroadcastOnUpdatePosition(client_id, x, y, z);
			}
		);
	}
	break;

	case EPacketProtocol::SC_LOOK_CHARACTER:
	{
		int32 client_id{};
		float r{}, y{}, p{};

		saga::ReceiveRotationPacket(packet_buffer, client_id, r, y, p);

		UE_LOG(LogSagaNetwork, Log, TEXT("[SagaGame] Client id %d: rotation(%f,%f,%f)"), client_id, r, y, p);

		CallFunctionOnGameThread([this, client_id, r, y, p]()
			{
				BroadcastOnUpdateRotation(client_id, r, y, p);
			}
		);
	}
	break;

	case EPacketProtocol::SC_UPDATE_CHARACTER:
	{
	}
	break;

	case EPacketProtocol::SC_RPC:
	{
		static ESagaRpcProtocol category{};
		static int32 user_id{};
		static int64 argument0{};
		static int32 argument1{};

		saga::ReceiveRpcPacket(packet_buffer, category, user_id, argument0, argument1);

		auto name = UEnum::GetValueAsString(category);

		UE_LOG(LogSagaNetwork, Log, TEXT("[SagaGame][RPC] %s(%lld, %d) from client %d"), *name, argument0, argument1, user_id);

		BroadcastOnRpc(category, user_id, argument0, argument1);
	}
	break;
	}
}
