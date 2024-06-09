export module Iconer.Net.IoCompletionPort;
export import Iconer.Net.ErrorCode;
export import Iconer.Net.IoContext;
import <cstdint>;
import <expected>;
import <span>;

export namespace iconer::net
{
	class Socket;

	struct [[nodiscard]] IoEvent
	{
		bool isSucceed;

		std::uint64_t eventId;

		unsigned long ioBytes;
		IoContext* ioContext;
	};

	class [[nodiscard]] IoCompletionPort final
	{
	public:
		explicit constexpr IoCompletionPort() noexcept = default;
		~IoCompletionPort() noexcept = default;

		bool Destroy() noexcept;
		bool Destroy(ErrorCode& error_code) noexcept;

		std::expected<void, iconer::net::ErrorCode> Register(iconer::net::Socket& socket, std::uintptr_t id) noexcept;
		bool TryRegister(net::Socket& socket, std::uintptr_t id, iconer::net::ErrorCode& error_code) noexcept;

		bool Schedule(IoContext& context, std::uintptr_t id, unsigned long infobytes = 0) noexcept;

		bool Schedule(IoContext* const context, std::uintptr_t id, unsigned long infobytes = 0) noexcept;

		bool Schedule(volatile IoContext& context, std::uintptr_t id, unsigned long infobytes = 0) noexcept;

		bool Schedule(volatile IoContext* const context, std::uintptr_t id, unsigned long infobytes = 0) noexcept;

		[[nodiscard]] IoEvent WaitForIoResult() noexcept;
		[[nodiscard]] std::expected<void, iconer::net::ErrorCode> WaitForMultipleIoResults(std::span<IoEvent> dest, unsigned long max_count);

		[[nodiscard]]
		constexpr bool IsAvailable() const noexcept
		{
			return nullptr != myHandle;
		}
		
		[[nodiscard]]
		constexpr operator bool() const noexcept
		{
			return nullptr != myHandle;
		}

		using FactoryResult = std::expected<IoCompletionPort, net::ErrorCode>;
		[[nodiscard]] static FactoryResult Create() noexcept;
		[[nodiscard]] static FactoryResult Create(std::uint32_t concurrency_hint) noexcept;

		IoCompletionPort(IoCompletionPort&&) noexcept = default;
		IoCompletionPort& operator=(IoCompletionPort&&) noexcept = default;

	private:
		IoCompletionPort(void* handle) noexcept;

		IoCompletionPort(const IoCompletionPort&) = delete;
		IoCompletionPort& operator=(const IoCompletionPort&) = delete;

		void* myHandle;
	};
}
