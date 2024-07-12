module;
#define LIKELY   [[likely]]
#define UNLIKELY [[unlikely]]

module Iconer.Framework;
import Iconer.App.SendContext;
import Iconer.App.PacketSerializer;

void
ServerFramework::EventOnUserList(iconer::app::User& user, std::byte* data)
{
	iconer::app::Room* const room = user.GetRoom();

	if (nullptr != room) LIKELY
	{
		user.SendGeneralData(*AcquireSendContext(), room->MakeMemberListPacket());
	};
}

void
ServerFramework::EventOnChangeTeam(iconer::app::User& user, std::byte* data)
{
	using enum iconer::app::TaskCategory;

	iconer::app::Room* const room = user.GetRoom();

	if (nullptr != room) LIKELY
	{
		auto & ctx = user.roomContext;

		const auto team_id = static_cast<std::int8_t>(*data);

		if (user.IsConnected() and ctx->TryChangeOperation(None, OpNotifyTeam)) LIKELY
		{
			if (team_id == 0) // red team
			{
				room->SetMemberTeam(user, true);
			}
			else if (team_id == 1) // blue team
			{
				room->SetMemberTeam(user, false);
			}
			else UNLIKELY
			{
				ctx->TryChangeOperation(OpNotifyTeam, None);
				return;
			};

			if (not myTaskPool.Schedule(ctx, user.GetID(), team_id)) UNLIKELY
			{
				ctx->TryChangeOperation(OpNotifyTeam, None);
			};
		};
	};
}

void
ServerFramework::EventOnNotifyTeamChanged(iconer::app::User& user, std::uint32_t team_id)
{
	using enum iconer::app::PacketProtocol;
	using enum iconer::app::TaskCategory;

	auto& ctx = user.roomContext;

	iconer::app::Room* const room = user.GetRoom();

	if (nullptr != room) LIKELY
	{
		// broadcast
		room->Foreach(
			[sender = AcquireSendContext()
			, id = static_cast<std::int32_t>(user.GetID())
			, team = (1 == team_id) ? (char)0 : (char)1](iconer::app::User& mem)
			{
				auto [pk, length] = iconer::app::Serialize(SC_SET_TEAM, id, team);

				sender->myBuffer = std::move(pk);

				mem.SendGeneralData(*sender, static_cast<size_t>(length));
			}
		);
	};

	ctx->TryChangeOperation(OpNotifyTeam, None);
}

void
ServerFramework::EventOnGameStartSignal(iconer::app::User& user, std::byte* data)
{
	using enum iconer::app::PacketProtocol;
	using enum iconer::app::TaskCategory;

	iconer::app::Room* const room = user.GetRoom();

	if (nullptr != room) LIKELY
	{
		auto & ctx = user.mainContext;

		if (ctx->TryChangeOperation(OpSignIn, OpCreateGame))
		{
			if (not myTaskPool.Schedule(ctx, user.GetID(), static_cast<unsigned long>(room->GetID())))
			{
				ctx->TryChangeOperation(OpCreateGame, OpSignIn);
			}
		}
	}
}

void
ServerFramework::EventOnMakeGame(iconer::app::User& user)
{
	auto& master_ctx = user.mainContext;

	using enum iconer::app::PacketProtocol;
	using enum iconer::app::TaskCategory;

	iconer::app::Room* const room = user.GetRoom();
	bool cancelled = false;

	if (nullptr != room) LIKELY
	{
		if (room->TryStartGame()) LIKELY
		{
			room->startingMemberProceedCount = 0;
			room->gameLoadedMemberProceedCount = 0;

			if (0 < room->GetMemberCount()) LIKELY
			{
				master_ctx->TryChangeOperation(OpCreateGame, OpSignIn);

				room->Foreach([&](iconer::app::User& mem)
					{
						auto& ctx = mem.mainContext;

						if (ctx->TryChangeOperation(OpSignIn, OpSpreadGameTicket)) LIKELY
						{
							const auto mem_room = mem.GetRoom();

							if (nullptr == mem_room) UNLIKELY
							{
								ctx->TryChangeOperation(OpSpreadGameTicket, None);

								cancelled = true;
							}
							else if (not mem.IsConnected()) UNLIKELY
							{
								ctx->TryChangeOperation(OpSpreadGameTicket, None);
								mem.myRoom = nullptr;

								cancelled = true;
							}
							else if (mem_room != room) UNLIKELY
							{
								ctx->TryChangeOperation(OpSpreadGameTicket, OpSignIn);

								cancelled = true;
							}
							else if (cancelled) UNLIKELY
							{
							}
							else LIKELY
							{
								mem.SendGeneralData(ctx, ctx->GetGameReadyPacketData());
							};
						};

						room->startingMemberProceedCount.fetch_add(1);
					}
				);
			}
			else UNLIKELY
			{
				cancelled = true;
			};
		};
	};

	if (cancelled) UNLIKELY
	{
		room->Foreach([&](iconer::app::User& mem)
			{
				auto& ctx = mem.mainContext;

				if (user.IsConnected())
				{
					ctx->TryChangeOperation(OpCreateGame, OpSignIn);
				}
				else
				{
					ctx->TryChangeOperation(OpCreateGame, None);
				}
			}
		);

		room->CancelStartGame();
	};
}

void
ServerFramework::EventOnSpreadGameTickets(iconer::app::User& user)
{
	iconer::app::Room* const room = user.GetRoom();

	if (nullptr != room) LIKELY
	{
		room->startingMemberProceedCount.fetch_add(1);
	};
}

void
ServerFramework::EventOnGameReadySignal(iconer::app::User& user, std::byte* data)
{
	using enum iconer::app::RoomState;

	iconer::app::Room* const room = user.GetRoom();

	if (nullptr != room) LIKELY
	{
		auto & cnt = room->gameLoadedMemberProceedCount;

		cnt.fetch_add(1);

		if (room->GetMemberCount() <= cnt)
		{
			iconer::app::RoomState state = PrepareGame;

			if (room->myState.compare_exchange_strong(state, Gaming))
			{
				room->Foreach([&](iconer::app::User& mem)
					{
						auto& ctx = mem.mainContext;

						mem.SendGeneralData(ctx, ctx->GetGameReadyPacketData());
					}
				);
			}
			else
			{
				// TODO
			}
		}
	};
}
