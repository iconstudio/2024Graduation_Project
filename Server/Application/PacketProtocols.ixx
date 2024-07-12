export module Iconer.App.PacketProtocol;
import <cstdint>;
import <limits>;

export namespace iconer::app
{
	enum class [[nodiscard]] PacketProtocol : std::uint8_t
	{
		UNKNOWN = 0,

		CS_SIGNIN,
		CS_SIGNOUT,
		CS_SIGNUP,					// 3

		CS_REQUEST_VERSION,
		CS_REQUEST_ROOMS,
		CS_REQUEST_USERS,			// 6

		CS_ROOM_CREATE,
		CS_ROOM_DESTROY,
		CS_ROOM_JOIN,				// 9
		CS_ROOM_MATCH,
		CS_ROOM_LEAVE,

		CS_SET_TEAM, // 12
		CS_READY, // 13
		CS_GAME_START, // 방장, 14
		CS_GAME_LOADED, // 
		CS_MY_POSITION, // 좌표
		CS_MY_TRANSFORM, // 좌표, 회전각
		CS_RPC, // 
		CS_GAME_EXIT,

		CS_CHAT,

		SC_SIGNIN_SUCCESS = 120,
		SC_SIGNIN_FAILURE,
		SC_SIGNUP_SUCCESS,
		SC_SIGNUP_FAILURE,

		SC_SERVER_INFO,
		SC_RESPOND_ROOMS,
		SC_RESPOND_USERS,
		SC_RESPOND_VERSION,

		SC_ROOM_CREATED_BY_SELF,
		SC_ROOM_CREATED,
		SC_ROOM_CREATE_FAILED,
		SC_ROOM_DESTROYED,
		SC_ROOM_JOINED,
		SC_ROOM_JOIN_FAILED,
		SC_ROOM_LEFT,
		SC_ROOM_MEMBER_LEFT,

		SC_SET_TEAM,
		SC_RPC,
		SC_GAME_START,
		SC_GAME_GETTING_READY,
		SC_FAILED_GAME_START,

		SC_GAME_VICTORY,
		SC_GAME_DEFEAT,

		SC_CREATE_PLAYER,
		SC_CREATE_OBJET,
		SC_MOVE_CHARACTER,
		SC_LOOK_CHARACTER,
		SC_UPDATE_CHARACTER,
		SC_REMOVE_CHARACTER,

		SC_PLAY_SFX,
		SC_PLAY_MUSIC,
		SC_ANIMATION_START,
		SC_CREATE_VFX,

		SC_CHAT,
	};
}
