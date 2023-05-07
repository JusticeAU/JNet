namespace JNet
{
	enum class JNetPacketType
	{
		UAuth,
		BSRegister,
		BSUpdate,
		MSRedirect,
		MSError
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
		int playerCount;
		int playerCapacity;
		int sessionCount;
		bool open;
	};

	struct MasterServerRedirect
	{
		JNetPacketType type = JNetPacketType::MSRedirect;
		char name[64];
		char hostname[64];
		int port;
	};
	struct MasterServerErrorMessage
	{
		JNetPacketType type = JNetPacketType::MSError;
		int errorID = 0;
		char message[256];
	};
}