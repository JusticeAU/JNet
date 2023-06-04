#pragma once

#include <vector>
#include <string>
#include <chrono>

struct _ENetHost;
struct _ENetPeer;
struct _ENetEvent;

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
			_ENetPeer* peer = nullptr;
			string name = "";
		};
		struct ConnectedGameSession
		{
			_ENetPeer* peer = nullptr;
			string name = "";
			string address = "";
			unsigned int port = 0;
		};
	private:
		// Balanced Server Info
		_ENetHost* m_ENetBalancedServerClient = nullptr;
		
		string m_myName = "";
		string m_myAddress = "";
		unsigned short m_myPort = 0;

		unsigned int m_playerCount = 0;
		unsigned int m_playerCapacity = 0;
		unsigned int m_sessionCount = 0;
		
		vector<ConnectedPeer> m_connectedPeers;

		// Master Server Info
		_ENetHost* m_ENetMasterServerClient = nullptr;
		_ENetPeer* m_ENetMasterServerPeer = nullptr;
		
		string m_masterServerAddress = "";
		unsigned short m_masterServerPort = 0;
		
		unsigned int m_MasterServerCheckInSeconds = 5;
		std::chrono::time_point<std::chrono::system_clock> m_MasterServerLastCheckInTime;
		bool m_connectedToMasterServer = false;
		bool m_openForConnections = false;

		// Connected Game Sessions
		_ENetHost* m_ENetGameSessionClient = nullptr;
		unsigned short m_myPortGS = 0;
		vector<ConnectedGameSession> m_connectedGameSessions;
		vector<string> m_countryCodes;

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
		void AddCountryCode(string code);
		void SendCountryCodesToMasterServer();

		void (*m_MasterServerConnectCallBack)(_ENetEvent*) = nullptr;
		void (*m_MasterServerPacketCallBack)(_ENetEvent*) = nullptr;
		void (*m_MasterServerDisconnectCallBack)(_ENetEvent*) = nullptr;

		void (*m_GameSessionConnectCallBack)(_ENetEvent*) = nullptr;
		void (*m_GameSessionPacketCallBack)(_ENetEvent*) = nullptr;
		void (*m_GameSessionDisconnectCallBack)(_ENetEvent*) = nullptr;

		void (*m_ClientConnectCallBack)(_ENetEvent*) = nullptr;
		void (*m_ClientPacketCallBack)(_ENetEvent*) = nullptr;
		void (*m_ClientDisconnectCallBack)(_ENetEvent*) = nullptr;
	};
}

