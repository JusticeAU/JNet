namespace JNet
{
	enum class JNetPacketType // enums 0-49 are reserved for JNet. Start your custom types from 50.
	{
		UAuth,
		URequestForGS,
		BSRegister,
		BSUpdate,
		BSGameServerInfo,
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

	struct UserAuth
	{
		JNetPacketType type = JNetPacketType::UAuth;
		char username[64] = "";
		char password[64] = "";;
	};

	struct UserRequestForGameSession
	{
		JNetPacketType type = JNetPacketType::URequestForGS;
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
		JNetPacketType type = JNetPacketType::BSGameServerInfo;
		char name[64] = "";;
		char address[64] = "";;
		int port = 0;
	};

	struct MasterServerRedirect
	{
		JNetPacketType type = JNetPacketType::MSRedirect;
		char name[64] = "";;
		char hostname[64] = "";;
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