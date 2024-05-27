module;
#include <string_view>

#define SEND(user_var, method, ...) \
auto [io, ctx] = ((user_var).method)(__VA_ARGS__); \
if (not io) \
{ \
	ctx.Complete(); \
}
#define SEND_RESULT(captures, user_var, method, ...) \
[captures](){ \
	auto [io, ctx] = ((user_var).method)(__VA_ARGS__); \
	if (not io) \
	{ \
		ctx.Complete(); \
		return false; \
	} \
	return true; \
}()

module Demo.Framework;
import Iconer.Application.Room;
import Iconer.Application.Rpc;

using namespace iconer::app;

bool
demo::Framework::OnCreatingCharacters(Room& room)
{
	auto number = room.GetMembersCount();
	if (number == 0)
	{
		return false;
	}

	auto& cnt_ref = room.proceedMemberCount;

	room.ForEach
	(
		[&](User& member)
		{
			auto [io, ctx] = member.SendCreateCharactersPacket();
			if (not io)
			{
				ctx.Complete();
			}

			cnt_ref.FetchAdd(1);
		}
	);

#if false
	if (cnt == 0)
	{
		return false;
	}
#else
	if (0 < cnt_ref)
	{
		//auto target_time = std::chrono::system_clock::now() + std::chrono::minutes{ 3 };

		//room.gameTimer = std::chrono::system_clock::to_time_t(target_time);

		return true;
	}
#endif

	return false;
}

void
demo::Framework::OnFailedToCreateCharacters(Room& room)
noexcept
{

}

bool
demo::Framework::OnRpc(IContext* ctx, const IdType& user_id)
{
	auto rpc_ctx = std::launder(static_cast<RpcContext*>(ctx));
	if (nullptr == rpc_ctx)
	{
		delete rpc_ctx;

		return false;
	}

	const IdType room_id = rpc_ctx->roomId;
	if (room_id <= -1)
	{
		delete rpc_ctx;

		return false;
	}

	auto room = FindRoom(room_id);
	if (nullptr == room)
	{
		delete rpc_ctx;

		return false;
	}

	if (room->GetMembersCount() == 0)
	{
		delete rpc_ctx;

		return false;
	}

	auto user = FindUser(user_id);
	if (nullptr == user)
	{
		delete rpc_ctx;

		return false;
	}

	auto& arg0 = rpc_ctx->firstArgument;
	auto& arg1 = rpc_ctx->secondArgument;

	using enum RpcProtocol;

	switch (rpc_ctx->rpcCategory)
	{
	case RPC_MAIN_WEAPON:
	{
		// arg0: kind of weapon
		// 0: lightsaber
		// 1: watergun
		// 2: hammer
		user->myWeaponId.Store(static_cast<std::uint8_t>(arg0));

		// broadcast his weapon
		room->ForEach
		(
			[&](User& member)
			{
				// NOTICE: RPC_MAIN_WEAPON(arg0, 0)
				SEND(member, SendRpcPacket, user_id, rpc_ctx->rpcCategory, arg0, 0);
			}
		);

		myLogger.Log(L"\tUser {} changed weapon to {}\n", user_id, arg0);
	}
	break;

	case RPC_BEG_RIDE:
	{
		// arg0: index of guardian
		if (arg0 < 0 or 3 <= arg0)
		{
			myLogger.LogError(L"\tUser {} tells wrong Guardian {}\n", user_id, arg0);
			break;
		}

		auto& guardian = room->sagaGuardians[arg0];

		if (guardian.CanRide())
		{
			myLogger.Log(L"\tUser {} would ride the Guardian {}\n", user_id, arg0);
		}
		else
		{
			myLogger.LogWarning(L"\tUser {} cannot ride the Guardian {}\n", user_id, arg0);
		}
	}
	break;

	case RPC_END_RIDE:
	{
		// arg0: index of guardian
		if (arg0 < 0 or 3 <= arg0)
		{
			myLogger.LogError(L"\tUser {} tells wrong Guardian {}\n", user_id, arg0);
			break;
		}

		auto& guardian = room->sagaGuardians[arg0];

		if (0 < user->myHealth and guardian.CanRide())
		{
			myLogger.Log(L"\tUser {} would ride the Guardian {}\n", user_id, arg0);
		}
		else
		{
			myLogger.LogWarning(L"\tUser {} cannot ride the Guardian {}\n", user_id, arg0);
		}
	}
	break;

	case RPC_BEG_ATTACK_0:
	{
		//if (0 < user->myHealth)
		{
			room->ForEach
			(
				[rpc_ctx, user_id](User& member)
				{
					SEND(member, SendRpcPacket, user_id, rpc_ctx->rpcCategory, rpc_ctx->firstArgument, rpc_ctx->secondArgument);
				}
			);
		}
	}
	break;

	case RPC_DMG_PLYER:
	{

	}
	break;

	case RPC_DMG_GUARDIAN:
	{

	}
	break;

	case RPC_DMG_GUARDIANS_PART:
	{

	}
	break;

	default:
	{
		room->ForEach
		(
			[rpc_ctx, user_id](User& member)
			{
				SEND(member, SendRpcPacket, user_id, rpc_ctx->rpcCategory, rpc_ctx->firstArgument, rpc_ctx->secondArgument);
			}
		);
	}
	break;
	}

	//rpc_ctx->lastOperation = AsyncOperations::OpCleanRpc;
	//(void)Schedule(rpc_ctx, 0);
	delete rpc_ctx;

	return true;
}

void
demo::Framework::OnSentRpc(iconer::app::IContext* ctx)
{
	auto rpc = std::launder(static_cast<RpcContext*>(ctx));
	delete rpc;
}

void
demo::Framework::OnCleanRpc(iconer::app::IContext* ctx)
{
	// returns the rpc context object
	auto rpc_ctx = std::launder(static_cast<RpcContext*>(ctx));
	rpc_ctx->lastOperation = AsyncOperations::OpRpc;

	auto it = lastRpcIterator;
	while (true)
	{
		for (; it != everyRpcContexts.end(); ++it)
		{
			auto& stored_ctx = *it;

			RpcContext* null = nullptr;
			if (stored_ctx.compare_exchange_strong(null, rpc_ctx))
			{
				// try to update the last iterator
				bool falsy = false;
				if (lastRpcLock.compare_exchange_strong(falsy, true))
				{
					lastRpcIterator = std::move(it);

					bool truthy = true;
					while (not lastRpcLock.compare_exchange_strong(truthy, false));
				}

				return;
			}
		}

		it = everyRpcContexts.begin();
	}
}

bool
demo::Framework::OnUpdatingRoom(iconer::app::Room& room)
{
	if (room.GetState() == iconer::app::RoomStates::InGame)
	{
		//auto list = room.

		return true;
	}
	else
	{
		return false;
	}
}

void
demo::Framework::OnFailedToUpdateRoom(iconer::app::Room& room)
noexcept
{
	if (room.GetMembersCount() == 0)
	{
		if (room.TryCancelBeginGame())
		{
			room.Cleanup();
		}
	}
	else if (room.GetState() == iconer::app::RoomStates::InGame)
	{

	}
	else
	{

	}
}
