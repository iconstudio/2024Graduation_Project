module Iconer.Framework;

import Iconer.Net;

void
ServerFramework::Cleanup()
{
	myTaskPool.Cleanup();
	userManager.Cleanup();
	roomManager.Cleanup();

	super::Cleanup();

	iconer::net::Cleanup();
}
