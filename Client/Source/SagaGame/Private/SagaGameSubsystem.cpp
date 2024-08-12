#include "SagaGameSubsystem.h"
#include <Containers/UnrealString.h>
#include <UObject/Object.h>
#include <Engine/World.h>
#include <GameFramework/Actor.h>
#include <Subsystems/SubsystemCollection.h>

#include "Player/SagaPlayerTeam.h"

#include "Saga/Network/SagaNetworkSubSystem.h"

USagaGameSubsystem::USagaGameSubsystem()
	: Super()
	, localPlayerSpawner()
	, RedTeamScore(), BluTeamScore()
	, RedTeamPinataBroken(), BluTeamPinataBroken()
{}

void
USagaGameSubsystem::Initialize(FSubsystemCollectionBase& collection)
{

}

void
USagaGameSubsystem::Deinitialize()
{

}

bool
USagaGameSubsystem::ShouldCreateSubsystem(UObject* outer)
const
{
	return true;
}

USagaGameSubsystem*
USagaGameSubsystem::GetSubSystem(const UWorld* world)
{
	const auto singleton = world->GetGameInstance();

	return singleton->GetSubsystem<USagaGameSubsystem>();
}

void
USagaGameSubsystem::AssignLocalPlayerSpawner(AActor* spawner)
noexcept
{
	localPlayerSpawner = spawner;
}

void
USagaGameSubsystem::SetScore(ESagaPlayerTeam team, int32 score)
noexcept
{
	const auto world = GetWorld();
	const auto net = USagaNetworkSubSystem::GetSubSystem(world);

	if (net->IsOfflineMode() or (net->IsConnected() and 0 < net->GetCurrentRoomId()))
	{
		lastLocalPlayerTeam = net->GetLocalUserTeam();
	}

	if (team == ESagaPlayerTeam::Red)
	{
		RedTeamScore = score;
	}
	else
	{
		BluTeamScore = score;
	}
}

void
USagaGameSubsystem::AddScore(ESagaPlayerTeam team, int32 score)
noexcept
{
	const auto world = GetWorld();
	const auto net = USagaNetworkSubSystem::GetSubSystem(world);

	if (net->IsOfflineMode() or (net->IsConnected() and 0 < net->GetCurrentRoomId()))
	{
		lastLocalPlayerTeam = net->GetLocalUserTeam();
	}

	if (team == ESagaPlayerTeam::Red)
	{
		RedTeamScore += score;
	}
	else
	{
		BluTeamScore += score;
	}
}

void
USagaGameSubsystem::SetWhoWonByPinata(int32 TeamPinataColor)
noexcept
{
	const auto world = GetWorld();
	const auto net = USagaNetworkSubSystem::GetSubSystem(world);

	if (net->IsOfflineMode() or (net->IsConnected() and 0 < net->GetCurrentRoomId()))
	{
		lastLocalPlayerTeam = net->GetLocalUserTeam();
	}

	if (TeamPinataColor == 0) //redteam's pinata broken
	{
		RedTeamPinataBroken = true;
	}
	else
	{
		BluTeamPinataBroken = true;
	}
}

FString
USagaGameSubsystem::GetWhoWon()
{
	const auto player_team = lastLocalPlayerTeam;

	if (RedTeamPinataBroken)
	{
		if (player_team == ESagaPlayerTeam::Red)
		{
			return TEXT("Lose"); // I'm in Red Team And My team Lose
		}
		else
		{
			return TEXT("Victory!"); // I'm in Blue Team And My team Win
		}

	}
	else if (BluTeamPinataBroken)
	{
		if (player_team == ESagaPlayerTeam::Red)
		{
			return TEXT("Victory!"); // I'm in Red Team And My team Win
		}
		else
		{
			return TEXT("Lose"); // I'm in Blue Team And My team Lose
		}
	}
	else if (RedTeamScore > BluTeamScore)
	{
		if (player_team == ESagaPlayerTeam::Red)
		{
			return TEXT("Victory!"); // I'm in Red Team And My team Win
		}
		else
		{
			return TEXT("Lose"); // I'm Blue in Team And My team Lose
		}
	}
	else if (RedTeamScore < BluTeamScore)
	{
		if (player_team == ESagaPlayerTeam::Red)
		{
			return TEXT("Lose"); // I'm inE Red Team And My team Lose
		}
		else
		{
			return TEXT("Victory!"); // I'm in Blue Team And My team Win
		}
	}
	else
	{
		return TEXT("Draw"); // Draw
	}
}

const AActor*
USagaGameSubsystem::GetLocalPlayerSpawner()
const noexcept
{
	return localPlayerSpawner;
}

int32
USagaGameSubsystem::GetRedTeamScore()
const noexcept
{
	return RedTeamScore;
}

int32
USagaGameSubsystem::GetBlueTeamScore()
const noexcept
{
	return BluTeamScore;
}
