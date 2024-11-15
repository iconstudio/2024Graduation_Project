export module Iconer.App.User;
import Iconer.Utility.ReadOnly;
import Iconer.Utility.Delegate;
import Iconer.Net.ErrorCode;
import Iconer.Net.IoResult;
import Iconer.Net.TcpReceiver;
import Iconer.App.UserContext;
import Iconer.App.TaskContext;
import Iconer.App.ISession;
import Iconer.App.ConnectionContract;
import Iconer.App.RoomContract;
import Iconer.App.Settings;
import <memory>;
import <array>;
import <string>;
import <span>;
import <atomic>;

export namespace iconer::app
{
	class [[nodiscard]] SendContext;

	class [[nodiscard]] User : public ISession
	{
	private:
		//static inline constinit std::atomic<id_type> globalMemberTable[Settings::usersLimit * Settings::roomMembersLimit]{};

		static inline constexpr size_t recvBufferSize = 512;

		std::atomic_bool isConnected{};

	public:
		using super = ISession;
		using this_class = User;
		using id_type = super::id_type;

		//userRoomTable[std::array{ 0ull, 1ull }];
		//static inline constinit std::mdspan userRoomTable{ globalMemberTable, std::extents<size_t, Settings::usersLimit, Settings::roomMembersLimit>{} };

		std::wstring myName{};
		std::atomic<class Room*> myRoom{};

		iconer::util::ReadOnly<UserContext> mainContext;
		iconer::util::ReadOnly<TaskContext> recvContext{};
		iconer::util::ReadOnly<TaskContext> roomContext{};
		iconer::net::TcpReceiver myReceiver;

		iconer::util::Delegate<void, User*> onDisconnected{};

		explicit constexpr User(id_type id, iconer::net::Socket& socket) noexcept
			: super(id)
			, mainContext(std::in_place, id, this)
			, myReceiver(socket, recvBufferSize)
		{}

		void Cleanup();

		iconer::net::IoResult SendGeneralData(iconer::app::SendContext& ctx, std::size_t length);
		iconer::net::IoResult SendGeneralData(iconer::app::SendContext& ctx, std::span<const std::byte> data);
		iconer::net::IoResult SendGeneralData(iconer::app::SendContext& ctx, const std::byte* data, std::size_t length);
		iconer::net::IoResult SendGeneralData(iconer::app::TaskContext& ctx, std::span<const std::byte> data);
		iconer::net::IoResult SendGeneralData(iconer::app::TaskContext& ctx, const std::byte* data, std::size_t length);

		iconer::net::IoResult SendGeneralData(iconer::app::SendContext& ctx, std::size_t length) const;
		iconer::net::IoResult SendGeneralData(iconer::app::SendContext& ctx, std::span<const std::byte> data) const;
		iconer::net::IoResult SendGeneralData(iconer::app::SendContext& ctx, const std::byte* data, std::size_t length) const;
		iconer::net::IoResult SendGeneralData(iconer::app::TaskContext& ctx, std::span<const std::byte> data) const;
		iconer::net::IoResult SendGeneralData(iconer::app::TaskContext& ctx, const std::byte* data, std::size_t length) const;

		iconer::net::IoResult SendSignInPacket();
		iconer::net::IoResult SendFailedSignInPacket(iconer::app::ConnectionContract reason);
		iconer::net::IoResult SendRoomCreatedPacket(id_type room_id);
		iconer::net::IoResult SendFailedCreateRoomPacket(iconer::app::TaskContext& ctx, iconer::app::RoomContract reason);
		iconer::net::IoResult SendJoinedRoomPacket(id_type room_id);
		iconer::net::IoResult SendRoomJoinFailedPacket(iconer::app::RoomContract reason);
		iconer::net::IoResult SendRoomLeftPacket(std::span<const std::byte> data);

		[[nodiscard]] iconer::net::IoResult BeginOptainReceiveMemory();
		bool EndOptainReceiveMemory(bool flag) noexcept;

		[[nodiscard]]
		iconer::net::IoResult BeginReceive()
		{
			return myReceiver.BeginReceive(recvContext);
		}

		bool EndReceive(const std::uint32_t bytes) noexcept
		{
			return myReceiver.EndReceive(recvContext, bytes);
		}

		iconer::net::IoResult BeginClose();
		void EndClose();

		void SetConnected(bool flag) volatile noexcept
		{
			isConnected.store(flag, std::memory_order_release);
		}

		void PullReceivedData(const size_t& size)
		{
			myReceiver.PullReceivedData(size);
		}

		[[nodiscard]]
		std::unique_ptr<std::byte[]> AcquireReceivedData(const size_t& size)
		{
			return myReceiver.AcquireReceivedData(size);
		}

		[[nodiscard]]
		constexpr const iconer::net::Socket& GetSocket() const noexcept
		{
			return myReceiver.GetSocket();
		}

		[[nodiscard]]
		auto GetReceivedData(const size_t& buf_size) noexcept
		{
			return myReceiver.GetReceiveBuffer(buf_size);
		}

		[[nodiscard]]
		auto GetReceivedData() noexcept
		{
			return myReceiver.GetReceiveBuffer();
		}

		[[nodiscard]]
		const std::wstring& GetName() const noexcept
		{
			return myName;
		}

		[[nodiscard]]
		auto GetRoom() const noexcept
		{
			return myRoom.load();
		}

		[[nodiscard]]
		auto GetRoom() const volatile noexcept
		{
			return myRoom.load();
		}

		[[nodiscard]]
		bool IsConnected() const volatile noexcept
		{
			return isConnected.load(std::memory_order_acquire);
		}

		[[nodiscard]]
		friend constexpr bool operator==(const User& lhs, const User& rhs) noexcept
		{
			return lhs.GetID() == rhs.GetID();
		}

	private:
		User(const User&) = delete;
		User& operator=(const User&) = delete;
		User(User&&) = delete;
		User& operator=(User&&) = delete;
	};
}
