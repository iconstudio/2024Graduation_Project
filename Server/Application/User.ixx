export module Iconer.App.User;
import Iconer.Utility.ReadOnly;
import Iconer.Utility.Delegate;
import Iconer.Net.ErrorCode;
import Iconer.Net.Socket;
import Iconer.Net.TcpReceiver;
import Iconer.App.UserContext;
import Iconer.App.TaskContext;
import Iconer.App.ISession;
import Iconer.App.Settings;
import <memory>;
import <expected>;
import <array>;
import <string>;
import <span>;
import <mdspan>;
import <atomic>;

export namespace iconer::app
{
	class [[nodiscard]] Room;

	class [[nodiscard]] User : public ISession
	{

	private:
		//static inline constinit std::atomic<id_type> globalMemberTable[Settings::usersLimit * Settings::roomMembersLimit]{};

		std::atomic_bool isConnected{};
		std::atomic<Room*> myRoom{};

	public:
		using super = ISession;
		using this_class = User;
		using id_type = super::id_type;

		static inline constexpr size_t recvBufferSize = 512;

		//userRoomTable[std::array{ 0ull, 1ull }];
		//static inline constinit std::mdspan userRoomTable{ globalMemberTable, std::extents<size_t, Settings::usersLimit, Settings::roomMembersLimit>{} };

		std::string myName{};

		iconer::util::ReadOnly<UserContext> mainContext;
		iconer::util::ReadOnly<TaskContext> recvContext{};
		iconer::util::ReadOnly<TaskContext> roomContext{};
		iconer::net::TcpReceiver myReceiver;

		iconer::util::Delegate<void, User*> onDisconnected{};

		explicit constexpr User(id_type id, iconer::net::Socket&& socket) noexcept
			: super(id)
			, mainContext(std::in_place, id, this)
			, myReceiver(std::move(socket), recvBufferSize)
		{}

		void Cleanup();

		[[nodiscard]]
		std::expected<void, iconer::net::ErrorCode> BeginOptainReceiveMemory()
		{
			recvContext->SetOperation(TaskCategory::OpOptainRecvMemory);

			return myReceiver.BeginOptainMemory(recvContext);
		}

		bool EndOptainReceiveMemory(bool flag) noexcept
		{
			myReceiver.EndOptainMemory(recvContext);

			SetConnected(flag);

			if (flag)
			{
				return recvContext->TryChangeOperation(TaskCategory::OpOptainRecvMemory, TaskCategory::OpRecv);
			}
			else
			{
				return recvContext->TryChangeOperation(TaskCategory::OpOptainRecvMemory, TaskCategory::None);
			}
		}

		[[nodiscard]]
		std::expected<void, iconer::net::ErrorCode> BeginReceive()
		{
			return myReceiver.BeginReceive(recvContext);
		}

		bool EndReceive(const std::uint32_t bytes) noexcept
		{
			return myReceiver.EndReceive(recvContext, bytes);
		}

		std::expected<void, iconer::net::ErrorCode> BeginClose();
		void EndClose();

		void SetConnected(bool flag) volatile noexcept
		{
			isConnected.store(flag, std::memory_order_release);
		}

		[[nodiscard]]
		std::span<const std::byte> GetReceivedData() const noexcept
		{
			return myReceiver.GetReceiveBuffer();
		}

		[[nodiscard]]
		std::unique_ptr<std::byte[]> AcquireReceivedData(size_t size) const
		{
			auto span = myReceiver.GetReceiveBuffer();

			std::unique_ptr<std::byte[]> result = std::make_unique<std::byte[]>(size);

			std::memcpy(result.get(), span.data(), size);

			return std::move(result);
		}

		[[nodiscard]]
		const iconer::net::Socket& GetSocket() const noexcept
		{
			return myReceiver.mySocket;
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
	};
}
