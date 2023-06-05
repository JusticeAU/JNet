#include "JNetPackets.h"
#include <iostream>
#include <vector>
#include <string>
#include <map>

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
			RoundRobin,
			GeoLocation
		};

		void Start();
		void LoadGeoIPDatabase(string filename);

		void Process();

		void SetBalanceMode(BalanceMode mode) { m_balanceMode = mode; }
		void SetClientConnectionPort(unsigned int port) { m_ENetClientPort = port; }
		void SetBSConnectionPort(unsigned int port) { m_ENetBSPort = port; }
		struct BalancedServerReference
		{
			_ENetPeer* peer = nullptr;
			string name = "undefined";
			string address = "undefined";
			unsigned short port = 0;
			unsigned short pingPort = 0;
			std::vector<string> geoRoutingCountryCodes;
			
			// Matches BSUpdate Data
			int playerCount = 0;
			int playerCapacity = 0;
			int sessionCount = 0;
			bool open = false;
		};
	private:
		_ENetAddress* m_ENetClientAddress = nullptr;
		_ENetHost* m_ENetClientHost = nullptr;
		unsigned int m_ENetClientPort = 6000;

		_ENetAddress* m_ENetBSAddress = nullptr;
		_ENetHost* m_ENetBSHost = nullptr;
		unsigned int m_ENetBSPort = 6001;

		BalanceMode m_balanceMode = BalanceMode::RoundRobin;
		vector<BalancedServerReference> m_balancedServers;

		JNet::MasterServerRedirect GetBalancedServer(_ENetPeer* peer);

		void InterpretUserPacket(_ENetEvent& receivedEvent);
		void InterpretBalancedServerPacket(_ENetEvent& receivedEvent);

		// Balance Modes
		// Round Robin
		int m_balanceModeRRnextServer = 0;
		// Least Response
		void MakeClientPingAllServersAndConnect(_ENetPeer* peer);
		// Georouting
		struct octet
		{
			int min = 0;
			int max = 0;
			std::vector<octet> next;
			std::string country = "";
		};
		std::string GetCountryFromIP(int first, int second, int third, int forth);
		int GetIndexOfBalancedServerForCountry(string countrycode);
		std::vector<octet> m_geoRoutingIPDatabase;
		std::vector<string> m_geoRoutingCountryCodes;

	public:
		// Public callbacks for user to subscribe to
		void (*m_BalancedServerConnectCallBack)(_ENetEvent*) = nullptr;
		void (*m_BalancedServerPacketCallBack)(_ENetEvent*) = nullptr;
		void (*m_BalancedServerDisconnectCallBack)(_ENetEvent*) = nullptr;

		void (*m_ClientConnectCallBack)(_ENetEvent*) = nullptr;
		void (*m_ClientPacketCallBack)(_ENetEvent*) = nullptr;
		void (*m_ClientDisconnectCallBack)(_ENetEvent*) = nullptr;
	};
}
