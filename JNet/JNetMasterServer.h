#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

struct _ENetAddress;
struct _ENetHost;
struct _ENetEvent;

using std::vector;
using std::string;

namespace JNet
{
	class MasterServer
	{
	public:
		MasterServer();
		~MasterServer();

		struct BalancedServerReference
		{
			string name = "undefined";
			string address = "undefined";
			unsigned short port = 0;
		};
	private:
		_ENetAddress* m_address;
		_ENetHost* m_ENetServer;

		// round robin balance
		int m_nextServer = 0;
		vector<BalancedServerReference> m_balancedServers;
	public:
		void Initialize();
		void Run();
		void Process();
		
		void InterpretUserPacket(_ENetEvent& receivedEvent);
		void InterpretBalancedServerPacket(_ENetEvent& receivedEvent);
	};
}
