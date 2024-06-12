export module Iconer.Net.Socket;
import Iconer.Utility.TypeTraits;
import Iconer.Utility.Property;
export import :SocketOption;
export import :SocketCategory;
export import Iconer.Net.ErrorCode;
export import Iconer.Net.InternetProtocol;
export import Iconer.Net.IpAddressFamily;
import <cstddef>;
import <cstdint>;
import <utility>;
import <expected>;
import <span>;

export struct _OVERLAPPED;

export namespace iconer::net
{
	class IpAddress;
	class EndPoint;
	class IoContext;

	class [[nodiscard]] Socket
	{
	public:
		using SyncIoResult = std::expected<int, iconer::net::ErrorCode>;
		using IoResult = std::expected<void, iconer::net::ErrorCode>;

		static inline constexpr std::uintptr_t invalidHandle = -1;

		explicit Socket() noexcept = default;

		IoResult Bind(const iconer::net::IpAddress& address, std::uint16_t port) const noexcept;
		IoResult Bind(iconer::net::IpAddress&& address, std::uint16_t port) const noexcept;
		IoResult Bind(const iconer::net::EndPoint& endpoint) const noexcept;
		IoResult Bind(iconer::net::EndPoint&& endpoint) const noexcept;
		IoResult BindToAny(std::uint16_t port) const noexcept;
		IoResult BindToHost(std::uint16_t port) const noexcept;

		IoResult Open() const noexcept;
		IoResult Connect(const iconer::net::IpAddress& address, std::uint16_t port) const noexcept;
		IoResult Connect(iconer::net::IpAddress&& address, std::uint16_t port) const noexcept;
		IoResult Connect(const iconer::net::EndPoint& endpoint) const noexcept;
		IoResult Connect(iconer::net::EndPoint&& endpoint) const noexcept;
		IoResult ConnectToHost(std::uint16_t port) const noexcept;

		IoResult SetOption(SocketOption option, const void* opt_buffer, const size_t opt_size) const noexcept;

		[[nodiscard]]
		bool ReusableAddress() const noexcept;
		IoResult ReusableAddress(bool flag) const noexcept;

		[[nodiscard]]
		std::expected<Socket, iconer::net::ErrorCode> Accept() const noexcept;
		[[nodiscard]]
		std::expected<Socket, iconer::net::ErrorCode> Accept(iconer::net::EndPoint& endpoint) const noexcept;

		/// <summary>
		/// Reserve an acceptance of <paramref name="client"/>
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="context"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="context"/>
		/// <param name="client">- Client socket</param>
		/// <exception cref="std::system_error"/>
		IoResult BeginAccept(iconer::net::IoContext& context, Socket& client) const;
		/// <summary>
		/// Reserve an acceptance of <paramref name="client"/>
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="context"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="context"/>
		/// <param name="client">- Client socket</param>
		/// <param name="accept_buffer">- Received datas from beginning would be written here</param>
		/// <exception cref="std::system_error"/>
		IoResult BeginAccept(iconer::net::IoContext& context, Socket& client, std::span<std::byte> accept_buffer) const;
		/// <summary>
		/// Reserve an acceptance of <paramref name="client"/>
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="context"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="context"/>
		/// <param name="client">- Client socket</param>
		/// <exception cref="std::system_error"/>
		IoResult BeginAccept(iconer::net::IoContext* context, Socket& client) const;
		/// <summary>
		/// Reserve an acceptance of <paramref name="client"/>
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="context"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="context"/>
		/// <param name="client">- Client socket</param>
		/// <param name="accept_buffer">- Received datas from beginning would be written here</param>
		/// <exception cref="std::system_error"/>
		IoResult BeginAccept(iconer::net::IoContext* context, Socket& client, std::span<std::byte> accept_buffer) const;
		/// <summary>
		/// Finish the acceptance of <paramref name="client"/>
		/// <para>It should be called by client</para>
		/// <para>-------------------------------------------------------------------------------</para>
		/// </summary>
		/// <param name="listener">- The listener socket</param>
		IoResult EndAccept(Socket& listener) const noexcept;

		bool Close() const noexcept;
		bool Close(iconer::net::ErrorCode& error_code) const noexcept;
		bool AsyncClose(iconer::net::IoContext& context) const noexcept;
		bool AsyncClose(iconer::net::IoContext& context, iconer::net::ErrorCode& error_code) const noexcept;
		bool AsyncClose(iconer::net::IoContext* const context) const noexcept;
		bool AsyncClose(iconer::net::IoContext* const context, iconer::net::ErrorCode& error_code) const noexcept;

		/// <summary>
		/// Send data through this socket
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="memory">- The send data buffer</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		SyncIoResult Send(std::span<const std::byte> memory) const;
		/// <summary>
		/// Send data through this socket
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="memory">- The send data buffer</param>
		/// <param name="size">- Size of the send data buffer</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		SyncIoResult Send(std::span<const std::byte> memory, size_t size) const;
		/// <summary>
		/// Send data through this socket
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="memory">- A pointer to the send data buffer</param>
		/// <param name="size">- Size of the send data buffer</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		SyncIoResult Send(const std::byte* const& memory, size_t size) const;
		/// <summary>
		/// Send data through this socket
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="memory">- The send data buffer</param>
		/// <param name="outpin">- The failsafe output pin for error code</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		bool Send(std::span<const std::byte> memory, iconer::net::ErrorCode& outpin) const;
		/// <summary>
		/// Send data through this socket
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="memory">- The send data buffer</param>
		/// <param name="size">- Size of the send data buffer</param>
		/// <param name="outpin">- The failsafe output pin for error code</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		bool Send(std::span<const std::byte> memory, size_t size, iconer::net::ErrorCode& outpin) const;
		/// <summary>
		/// Send data through this socket
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="memory">- A pointer to the send data buffer</param>
		/// <param name="size">- Size of the send data buffer</param>
		/// <param name="outpin">- The failsafe output pin for error code</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		bool Send(const std::byte* const& memory, size_t size, iconer::net::ErrorCode& outpin) const;

		/// <summary>
		/// Send data through this socket
		/// <para><b>(Asynchronous IO)</b></para>
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="context">- The io context object</param>
		/// <param name="memory">- The send data buffer</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		IoResult Send(iconer::net::IoContext& context, std::span<const std::byte> memory) const;
		/// <summary>
		/// Send data through this socket
		/// <para><b>(Asynchronous IO)</b></para>
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="context">- The io context object</param>
		/// <param name="memory">- The send data buffer</param>
		/// <param name="size">- Size of the send data buffer</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		IoResult Send(iconer::net::IoContext& context, std::span<const std::byte> memory, size_t size) const;
		/// <summary>
		/// Send data through this socket
		/// <para><b>(Asynchronous IO)</b></para>
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="context">- The io context object</param>
		/// <param name="memory">- A pointer to the send data buffer</param>
		/// <param name="size">- Size of the send data buffer</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		IoResult Send(iconer::net::IoContext& context, const std::byte* const& memory, size_t size) const;
		bool Send(iconer::net::IoContext& context, std::span<const std::byte> memory, iconer::net::ErrorCode& outpin) const;
		bool Send(iconer::net::IoContext& context, std::span<const std::byte> memory, size_t size, iconer::net::ErrorCode& outpin) const;
		bool Send(iconer::net::IoContext& context, const std::byte* const& memory, size_t size, iconer::net::ErrorCode& outpin) const;

		/// <summary>
		/// Receive data into the memory buffer
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="memory">- The receive data buffer</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		SyncIoResult Receive(std::span<std::byte> memory) const;
		/// <summary>
		/// Receive data into the memory buffer
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="memory">- The receive data buffer</param>
		/// <param name="size">- Size of the receive data buffer</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		SyncIoResult Receive(std::span<std::byte> memory, size_t size) const;
		/// <summary>
		/// Receive data into the memory buffer
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="memory">- A pointer to the receive data buffer</param>
		/// <param name="size">- Size of the receive data buffer</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		SyncIoResult Receive(std::byte* memory, size_t size) const;
		/// <summary>
		/// Receive data into the memory buffer
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="memory">- The receive data buffer</param>
		/// <param name="outpin">- The failsafe output pin for error code</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		bool Receive(std::span<std::byte> memory, ErrorCode& outpin) const;
		/// <summary>
		/// Receive data into the memory buffer
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="memory">- The receive data buffer</param>
		/// <param name="size">- Size of the receive data buffer</param>
		/// <param name="outpin">- The failsafe output pin for error code</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		bool Receive(std::span<std::byte> memory, size_t size, ErrorCode& outpin) const;
		/// <summary>
		/// Receive data into the memory buffer
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="memory">- A pointer to the receive data buffer</param>
		/// <param name="size">- Size of the receive data buffer</param>
		/// <param name="outpin">- The failsafe output pin for error code</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		bool Receive(std::byte* memory, size_t size, ErrorCode& outpin) const;

		/// <summary>
		/// Receive data into the memory buffer
		/// <para><b>(Asynchronous IO)</b></para>
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="context">- The io context object</param>
		/// <param name="memory">- The receive data buffer</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		IoResult Receive(IoContext& context, std::span<std::byte> memory) const;
		/// <summary>
		/// Receive data into the memory buffer
		/// <para><b>(Asynchronous IO)</b></para>
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="memory">- The receive data buffer</param>
		/// <param name="size">- Size of the receive data buffer</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		IoResult Receive(IoContext& context, std::span<std::byte> memory, size_t size) const;
		/// <summary>
		/// Receive data into the memory buffer
		/// <para><b>(Asynchronous IO)</b></para>
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="context">- The io context object</param>
		/// <param name="memory">- A pointer to the receive data buffer</param>
		/// <param name="size">- Size of the receive data buffer</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		IoResult Receive(IoContext& context, std::byte* memory, size_t size) const;
		/// <summary>
		/// Receive data into the memory buffer
		/// <para><b>(Asynchronous IO)</b></para>
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="context">- The io context object</param>
		/// <param name="memory">- The receive data buffer</param>
		/// <param name="outpin">- The failsafe output pin for error code</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		bool Receive(IoContext& context, std::span<std::byte> memory, ErrorCode& outpin) const;
		/// <summary>
		/// Receive data into the memory buffer
		/// <para><b>(Asynchronous IO)</b></para>
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="context">- The io context object</param>
		/// <param name="memory">- The receive data buffer</param>
		/// <param name="size">- Size of the receive data buffer</param>
		/// <param name="outpin">- The failsafe output pin for error code</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		bool Receive(IoContext& context, std::span<std::byte> memory, size_t size, ErrorCode& outpin) const;
		/// <summary>
		/// Receive data into the memory buffer
		/// <para><b>(Asynchronous IO)</b></para>
		/// <para>-------------------------------------------------------------------------------</para>
		/// <para>throws <see cref="std::system_error"/> when <paramref name="memory"/> is <value>nullptr</value></para>
		/// </summary>
		/// <param name="context">- The io context object</param>
		/// <param name="memory">- A pointer to the receive data buffer</param>
		/// <param name="size">- Size of the receive data buffer</param>
		/// <param name="outpin">- The failsafe output pin for error code</param>
		/// <exception cref="std::overflow_error"/>
		/// <exception cref="std::system_error"/>
		bool Receive(IoContext& context, std::byte* memory, size_t size, ErrorCode& outpin) const;

		[[nodiscard]]
		constexpr const std::uintptr_t& GetNativeHandle() const& noexcept
		{
			return myHandle;
		}

		[[nodiscard]]
		constexpr std::uintptr_t&& GetNativeHandle() && noexcept
		{
			return std::move(myHandle);
		}

		[[nodiscard]]
		constexpr iconer::net::InternetProtocol GetInternetProtocol() const noexcept
		{
			return myProtocol;
		}

		[[nodiscard]]
		constexpr iconer::net::IpAddressFamily GetIpAddressFamily() const noexcept
		{
			return myFamily;
		}

		[[nodiscard]]
		constexpr explicit operator bool() const noexcept
		{
			return (0 != myHandle and invalidHandle != myHandle);
		}

		constexpr Socket(Socket&& other) noexcept
			: myHandle(std::exchange(other.myHandle, invalidHandle))
			, myProtocol(other.myProtocol)
			, myFamily(other.myFamily)
		{}

		constexpr Socket& operator=(Socket&& other) noexcept
		{
			myHandle = std::exchange(other.myHandle, invalidHandle);
			myProtocol = other.myProtocol;
			myFamily = other.myFamily;

			return *this;
		}

		[[nodiscard]]
		constexpr bool operator==(const Socket&) const noexcept = default;

		[[nodiscard]]
		static std::expected<Socket, iconer::net::ErrorCode> Create(iconer::net::SocketCategory type, iconer::net::InternetProtocol protocol, iconer::net::IpAddressFamily family) noexcept;

		[[nodiscard]]
		static bool TryCreate(iconer::net::SocketCategory type, iconer::net::InternetProtocol protocol, iconer::net::IpAddressFamily family, iconer::net::Socket& result, iconer::net::ErrorCode& outpin) noexcept;

		[[nodiscard]]
		static Socket CreateTcpSocket(iconer::net::SocketCategory type, iconer::net::IpAddressFamily family = iconer::net::IpAddressFamily::IPv4) noexcept;

		[[nodiscard]]
		static Socket CreateUdpSocket(iconer::net::SocketCategory type, iconer::net::IpAddressFamily family = iconer::net::IpAddressFamily::IPv4) noexcept;

		iconer::util::Property<bool, Socket> reuseAddress{ this, &Socket::ReuseAddressImplementation, false };

	private:
		std::uintptr_t myHandle = invalidHandle;
		iconer::net::InternetProtocol myProtocol = iconer::net::InternetProtocol::Unknown;
		iconer::net::IpAddressFamily myFamily = iconer::net::IpAddressFamily::Unknown;

		using AsyncAcceptFunction = std_function_t<int, std::uintptr_t, std::uintptr_t, void*, unsigned long, unsigned long, unsigned long, unsigned long*, struct _OVERLAPPED*>;

		using AsyncCloseFunction = std_function_t<int, std::uintptr_t, void*, unsigned long, unsigned long, struct _OVERLAPPED*, struct _TRANSMIT_FILE_BUFFERS*, unsigned long>;

		struct ErrorTransfer
		{
			constexpr std::expected<bool, iconer::net::ErrorCode> operator()(iconer::net::ErrorCode&& error_code) const noexcept
			{
				errorOutput = std::exchange(error_code, {});
				return false;
			}

			iconer::net::ErrorCode& errorOutput;
		};

		void ReuseAddressImplementation(bool flag) const noexcept;

		static inline constinit AsyncAcceptFunction asyncAcceptFnPtr = nullptr;
		static inline constinit AsyncCloseFunction asyncTransmitFnPtr = nullptr;

		Socket(std::uintptr_t sock, iconer::net::InternetProtocol protocol, iconer::net::IpAddressFamily family) noexcept;

		static void InternalFunctionInitializer(std::uintptr_t socket);

		Socket(const Socket&) = delete;
		Socket& operator=(const Socket&) = delete;
	};
}