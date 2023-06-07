#include "JNetPackets.h"
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <chrono>

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
		
		// This function assumes a CSV file with the following columns:
		// IPV4 Start, IPV4 End, Country code.
		// It assumes they are in order from lowest to highest. e.g.
		// 1.0.0.0,1.0.0.255,AU
		// 1.0.1.0,1.0.3.255,CN
		// 1.0.4.0,1.0.7.255,AU
		// and so on
		void LoadGeoIPDatabase(string filename);

		// Should be called regularly by your implementation.
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
		
		// An octet is used to dive in to the geo routing IP database.
		// It represents a section of an IP range covered by a particular area code.
		// For example, a range described as:
		// 1.0.4.0, 1.0.7.255, AU
		// The first octet has a min and max of 1, the second has a min and max of 0, the third has a min of 4 and a max of 7, and the fourth has a min of 0 and a max of 255
		// The 'next' vector contains all next possible octets.
		// The GetCountryFromIP function dives 4 octet layers deep to find the matching country code.
		// See LoadGeoIPDatabase function for more details.
		struct octet 
		{
			unsigned char min = 0;
			unsigned char max = 0;
			std::vector<octet> next;
			std::string country = "";
		};
		// Dives in to the IPDatabase to find the country code for the peers IP address. If no range covering the IP is found, the function returns "DEFAULT"
		std::string GetCountryFromIP(int first, int second, int third, int forth);
		// Iterates over the list of Balanced Servers searching for one that covers countrycode. If none is found, '0' is returned.
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
