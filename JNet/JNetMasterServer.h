#include "JNetPackets.h"
#include <iostream>
#include <vector>
#include <string>

struct _ENetAddress;
struct _ENetHost;
struct _ENetEvent;
struct _ENetPeer;

using std::vector;
using std::string;

namespace JNet
{
	class MasterServer
	{
	public:
		MasterServer();
		~MasterServer();

		enum class BalanceMode
		{
			LeastConnection,
			LeastResponseTime,
			RoundRobin
		};

		struct BalancedServerReference
		{
			_ENetPeer* peer = nullptr;
			string name = "undefined";
			string address = "undefined";
			unsigned short port = 0;
			unsigned short pingPort = 0;
			
			// Matches BSUpdate Data
			int playerCount = 0;
			int playerCapacity = 0;
			int sessionCount = 0;
			bool open = false;
		};
	private:
		_ENetAddress* m_ENetClientAddress;
		_ENetHost* m_ENetClientHost;
		_ENetAddress* m_ENetBSAddress;
		_ENetHost* m_ENetBSHost;


		BalanceMode m_balanceMode = BalanceMode::LeastConnection;

		// round robin balance
		int m_balanceModeRRnextServer = 0;
		vector<BalancedServerReference> m_balancedServers;
	public:
		void Initialize();

		void Process();

		JNet::MasterServerRedirect GetBalancedServer();
		
		void InterpretUserPacket(_ENetEvent& receivedEvent);
		void InterpretBalancedServerPacket(_ENetEvent& receivedEvent);

		void MakeClientPingAllServersAndConnect(_ENetPeer* peer);

		void (*m_BalancedServerConnectCallBack)(_ENetEvent*) = nullptr;
		void (*m_BalancedServerPacketCallBack)(_ENetEvent*) = nullptr;
		void (*m_BalancedServerDisconnectCallBack)(_ENetEvent*) = nullptr;

		void (*m_ClientConnectCallBack)(_ENetEvent*) = nullptr;
		void (*m_ClientPacketCallBack)(_ENetEvent*) = nullptr;
		void (*m_ClientDisconnectCallBack)(_ENetEvent*) = nullptr;
	};
}
