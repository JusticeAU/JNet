#pragma once

#include <vector>
#include <string>
#include <chrono>

struct _ENetHost;
struct _ENetPeer;

using std::vector;
using std::string;

namespace JNet
{
	class BalancedServer
	{
	public:
		BalancedServer();
		~BalancedServer();

		struct ConnectedPeer
		{
			_ENetPeer* peer;
			string name;
		};
		struct ConnectedGameSession
		{
			_ENetPeer* peer;
			string name;
			string address;
			unsigned int port;
		};
	private:
		// Balanced Server Info
		_ENetHost* m_ENetBalancedServerClient;
		
		string m_myName;
		string m_myAddress;
		unsigned short m_myPort;

		unsigned int m_playerCount;
		unsigned int m_playerCapacity;
		unsigned int m_sessionCount;
		
		vector<ConnectedPeer> m_connectedPeers;

		// Master Server Info
		_ENetHost* m_ENetMasterServerClient;
		_ENetPeer* m_ENetMasterServerPeer;
		
		string m_masterServerAddress;
		unsigned short m_masterServerPort;
		
		unsigned int m_MasterServerCheckInSeconds = 5;
		std::chrono::time_point<std::chrono::system_clock> m_MasterServerLastCheckInTime;
		bool m_connectedToMasterServer = false;
		bool m_openForConnections = false;

		// Connected Game Sessions
		_ENetHost* m_ENetGameSessionClient;
		unsigned short m_myPortGS;
		vector<ConnectedGameSession> m_connectedGameSessions;

	public:
		void Initialise();
		void SetMyConnectionInfo(string myName, string myAddress, unsigned int myPort, unsigned int myPortGS);
		
		void Update();
		void UpdateClients();

		void UpdateGameSessions();

		void SetMasterServer(string address, unsigned int port);
		void ConnectToMasterServer();
		void CheckInWithMasterServer();
		void UpdateMasterServer();
		void OpenForConnections();

		void CreateAGameSessionBoi();
		void CreateAGameSessionBoiDifferently();
	};
}

