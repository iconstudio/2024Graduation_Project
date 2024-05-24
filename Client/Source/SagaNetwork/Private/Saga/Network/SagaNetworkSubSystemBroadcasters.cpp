#include "Saga/Network/SagaNetworkSubSystem.h"

void
USagaNetworkSubSystem::BroadcastOnNetworkInitialized()
const
{
	UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnNetworkInitialized`"));

	if (OnNetworkInitialized.IsBound())
	{
		CallPureFunctionOnGameThread([this]()
			{
				OnNetworkInitialized.Broadcast(true);
			}
		);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnFailedToInitializeNetwork` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnFailedToInitializeNetwork()
const
{
	UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnFailedToInitializeNetwork`"));

	if (OnNetworkInitialized.IsBound())
	{
		CallPureFunctionOnGameThread([this]()
			{
				OnNetworkInitialized.Broadcast(false);
			}
		);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnFailedToInitializeNetwork` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnConnected()
const
{
	UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnConnected`"));

	if (OnConnected.IsBound())
	{
		CallPureFunctionOnGameThread([this]()
			{
				OnConnected.Broadcast();
			}
		);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnConnected` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnFailedToConnect(ESagaConnectionContract reason)
const
{
	UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnFailedToConnect`"));

	if (OnFailedToConnect.IsBound())
	{
		CallPureFunctionOnGameThread([this, reason]()
			{
				OnFailedToConnect.Broadcast(reason);
			}
		);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnFailedToConnect` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnDisconnected()
const
{
	UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnDisconnected`"));

	if (OnDisconnected.IsBound())
	{
		CallPureFunctionOnGameThread([this]()
			{
				OnDisconnected.Broadcast();
			}
		);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnDisconnected` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnSignedIn(int32 my_id, const FName& nickname)
const
{
	UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnSignedIn`"));

	if (OnSignedIn.IsBound())
	{
		CallPureFunctionOnGameThread([this, my_id, nickname]()
			{
				OnSignedIn.Broadcast(my_id, nickname);
			}
		);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnSignedIn` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnRoomCreated(int32 room_id)
const
{
	UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnRoomCreated`"));

	if (OnRoomCreated.IsBound())
	{
		CallPureFunctionOnGameThread([this, room_id]()
			{
				OnRoomCreated.Broadcast(room_id);
			}
		);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnRoomCreated` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnJoinedRoom(int32 user_id)
const
{
	UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnJoinedRoom`"));

	if (OnJoinedRoom.IsBound())
	{
		OnJoinedRoom.Broadcast(user_id);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnJoinedRoom` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnLeftRoomBySelf()
const
{
	UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnLeftRoomBySelf`"));

	if (OnLeftRoomBySelf.IsBound())
	{
		OnLeftRoomBySelf.Broadcast();
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnLeftRoomBySelf` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnLeftRoom(int32 id)
const
{
	UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnLeftRoom`"));

	if (OnLeftRoom.IsBound())
	{
		OnLeftRoom.Broadcast(id);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnLeftRoom` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnRespondVersion(const FString& version_string)
const
{
	UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnRespondVersion`"));

	if (OnRespondVersion.IsBound())
	{
		OnRespondVersion.Broadcast(version_string);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnRespondVersion` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnUpdateRoomList(const TArray<FSagaVirtualRoom>& list)
const
{
	UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnUpdateRoomList`"));

	if (OnUpdateRoomList.IsBound())
	{
		OnUpdateRoomList.Broadcast(list);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnUpdateRoomList` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnUpdateMembers(const TArray<FSagaVirtualUser>& list)
const
{
	UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnUpdateMembers`"));

	if (OnUpdateMembers.IsBound())
	{
		OnUpdateMembers.Broadcast(list);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnUpdateMembers` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnTeamChanged(int32 user_id, bool is_red_team)
const
{
	UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnTeamChanged`"));

	if (OnTeamChanged.IsBound())
	{
		CallPureFunctionOnGameThread([this, user_id, is_red_team]()
			{
				OnTeamChanged.Broadcast(user_id, is_red_team);
			}
		);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnTeamChanged` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnGetPreparedGame()
const
{
	UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnGetPreparedGame`"));

	if (OnGetPreparedGame.IsBound())
	{
		CallPureFunctionOnGameThread([this]()
			{
				OnGetPreparedGame.Broadcast();
			}
		);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnGetPreparedGame` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnFailedToStartGame(ESagaGameContract reason)
const
{
	UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnFailedToStartGame`"));

	if (OnFailedToStartGame.IsBound())
	{
		CallPureFunctionOnGameThread([this, reason]()
			{
				OnFailedToStartGame.Broadcast(reason);
			}
		);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnFailedToStartGame` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnStartGame()
const
{
	UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnStartGame`"));

	if (OnStartGame.IsBound())
	{
		CallPureFunctionOnGameThread([this]()
			{
				OnStartGame.Broadcast();
			}
		);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnStartGame` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnUpdatePosition(int32 user_id, float x, float y, float z)
const
{
	//UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnUpdatePosition`"));

	if (OnUpdatePosition.IsBound())
	{
		CallPureFunctionOnGameThread([this, user_id, x, y, z]()
			{
				OnUpdatePosition.Broadcast(user_id, x, y, z);
			}
		);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnUpdatePosition` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnUpdateRotation(int32 user_id, float r, float y, float p)
const
{
	//UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnUpdateRotation`"));

	if (OnUpdateRotation.IsBound())
	{
		CallPureFunctionOnGameThread([this, user_id, r, y, p]()
			{
				OnUpdateRotation.Broadcast(user_id, r, y, p);
			}
		);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnUpdateRotation` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnCreatingCharacter(int32 user_id, EUserTeam team, EPlayerWeapon weapon)
const
{
	UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnCreatingCharacter`"));

	if (OnCreatingCharacter.IsBound())
	{
		CallPureFunctionOnGameThread([this, user_id, team, weapon]()
			{
				OnCreatingCharacter.Broadcast(user_id, team, weapon);
			}
		);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnCreatingCharacter` was not bound"));
	}
}

void
USagaNetworkSubSystem::BroadcastOnRpc(ESagaRpcProtocol cat, int32 user_id, int64 arg0, int32 arg1)
const
{
	//UE_LOG(LogSagaNetwork, Log, TEXT("Brodcasting `OnRpc`"));

	if (OnRpc.IsBound())
	{
		CallPureFunctionOnGameThread([this, cat, user_id, arg0, arg1]()
			{
				OnRpc.Broadcast(cat, user_id, arg0, arg1);
			}
		);
	}
	else
	{
		UE_LOG(LogSagaNetwork, Warning, TEXT("`OnRpc` was not bound"));
	}
}
