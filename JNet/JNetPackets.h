namespace JNet
{
	enum class JNetPacketType
	{
		UAuth,
		URequestForGS,
		BSRegister,
		BSUpdate,
		BSGameServerInfo,
		MSRedirect,
		GSRegister,
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
		char username[64];
		char password[64];
	};

	struct UserRequestForGameSession
	{
		JNetPacketType type = JNetPacketType::URequestForGS;
	};

	struct BalancedServerRegister
	{
		JNetPacketType type = JNetPacketType::BSRegister;
		char name[64];
		char hostname[64];
		int port;
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
		char name[64];
		char address[64];
		int port;
	};

	struct MasterServerRedirect
	{
		JNetPacketType type = JNetPacketType::MSRedirect;
		char name[64];
		char hostname[64];
		int port;
	};

	struct GameSessionRegister
	{
		JNetPacketType type = JNetPacketType::GSRegister;
		char name[64];
		char hostname[64];
		int port;
	};

	struct ErrorMessage
	{
		JNetPacketType type = JNetPacketType::Error;
		int errorID = 0;
		char message[256];
	};
}