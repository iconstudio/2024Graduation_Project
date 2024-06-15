export module Iconer.Net.IFramework;
import Iconer.Utility.Delegate;
import Iconer.Net.ErrorCode;
import Iconer.Net.Socket;
import <expected>;

export namespace iconer::net
{
	class IFramework
	{
	public:
		iconer::util::Delegate<void> eventOnInitialied;
		iconer::util::Delegate<void> eventOnPostInitialied;
		iconer::util::Delegate<void> eventOnPreStarted;
		iconer::util::Delegate<void> eventOnPostStarted;
		iconer::util::Delegate<void> eventOnDestructed;

		constexpr IFramework() = default;
		virtual constexpr ~IFramework() = default;

		virtual std::expected<void, iconer::net::ErrorCode> Initialize();
		virtual void Startup();
		virtual void PostStartup();
		virtual void Cleanup();

	protected:
		iconer::net::Socket listenSocket;

	private:
		IFramework(const IFramework&) = delete;
		IFramework& operator=(const IFramework&) = delete;
		IFramework(IFramework&&) = default;
		IFramework& operator=(IFramework&&) = default;
	};
}