#pragma once
namespace JNet
{
	enum class JNetPacketType // enums 0-49 are reserved for JNet. Start your custom types from 50.
	{
		ClientAuth,
		ClientRequestForGS,
		ClientRequestForAllGS,
		ClientDisconnect,
		BSRegister,
		BSUpdate,
		BSGameSessionInfo,
		BSConnectToGameServer,
		MSRedirect,
		MSCheckPingStart,
		MSCheckPingServer,
		GSRegister,
		Ping,
		Error
	};

	// Cast packets to this to see what they are.
	struct JNetPacket
	{
		JNetPacketType type;
	};

	struct ClientAuth
	{
		JNetPacketType type = JNetPacketType::ClientAuth;
		char username[64] = "";
		char password[64] = "";;
	};

	struct ClientRequestForGameSession
	{
		JNetPacketType type = JNetPacketType::ClientRequestForGS;
	};

	struct BalancedServerRegister
	{
		JNetPacketType type = JNetPacketType::BSRegister;
		char name[64] = "";;
		char hostname[64] = "";;
		int port = 0;

		// Matches BS Update Data
		int playerCount = 0;
		int playerCapacity = 0;
		int sessionCount = 0;
		bool open = false;
	};

	struct BalancedServerUpdate
	{
		JNetPacketType type = JNetPacketType::BSUpdate;
		int playerCount = 0;
		int playerCapacity = 0;
		int sessionCount = 0;
		bool open = false;
	};

	struct BalancedServerGameSessionInfo
	{
		JNetPacketType type = JNetPacketType::BSGameSessionInfo;
		char name[64] = "";
		char address[64] = "";
		int port = 0;
		int players = 0;
	};

	struct BalancedServerConnectToGameSession
	{
		JNetPacketType type = JNetPacketType::BSConnectToGameServer;
		char name[64] = "";
		char address[64] = "";
		int port = 0;
	};

	struct MasterServerRedirect
	{
		JNetPacketType type = JNetPacketType::MSRedirect;
		char name[64] = "";
		char hostname[64] = "";
		int port = 0;
	};

	struct MasterServerCheckPingStart
	{
		JNetPacketType type = JNetPacketType::MSCheckPingStart;
		int quantity = 0;
	};

	struct MasterServerCheckPingServer
	{
		JNetPacketType type = JNetPacketType::MSCheckPingServer;
		char name[64] = "";
		char hostname[64] = "";
		int port = 0;
	};

	struct GameSessionRegister
	{
		JNetPacketType type = JNetPacketType::GSRegister;
		char name[64] = "";
		char hostname[64] = "";
		int port = 0;
	};

	struct Ping
	{
		JNetPacketType type = JNetPacketType::Ping;
	};

	struct ErrorMessage
	{
		JNetPacketType type = JNetPacketType::Error;
		int errorID = 0;
		char message[256] = "";;
	};
}