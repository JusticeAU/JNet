#pragma once

#include "JNetPackets.h"
#include <string>
#include <list>

using std::string;

struct _ENetHost;
struct _ENetPeer;
struct _ENetEvent;

namespace JNet
{
	class Client
	{
		struct MasterServerReference
		{
			string address = "";
			unsigned short port = 0;
		};

		struct BalancedServerReference
		{
			string name = "";
			string address = "";
			unsigned short port = 0;
		};

		struct GameSessionReference
		{
			string name = "";
			string address = "";
			unsigned short port = 0;
		};

	public:
		Client();
		~Client();

		// Callbacks to subscribe to
		void (*m_MasterServerConnectCallBack)(_ENetEvent*) = nullptr;
		void (*m_MasterServerPacketCallBack)(_ENetEvent*) = nullptr;
		void (*m_MasterServerDisconnectCallBack)(_ENetEvent*) = nullptr;

		void (*m_BalancedServerConnectCallBack)(_ENetEvent*) = nullptr;
		void (*m_BalancedServerPacketCallBack)(_ENetEvent*) = nullptr;
		void (*m_BalancedServerDisconnectCallBack)(_ENetEvent*) = nullptr;

		void (*m_GameSessionConnectCallBack)(_ENetEvent*) = nullptr;
		void (*m_GameSessionPacketCallBack)(_ENetEvent*) = nullptr;
		void (*m_GameSessionDisconnectCallBack)(_ENetEvent*) = nullptr;

		// Game Session Management
		std::list<JNet::BalancedServerGameSessionInfo> m_GameSessions;

	private:
		string	m_masterServerAddress = "";
		int		m_masterServerPort = 0;

		_ENetHost* m_ENetMasterServerClient = nullptr;
		_ENetPeer* m_ENetMasterServerPeer = nullptr;;
		bool m_shouldCloseMasterServerHost = false;

		// Balanced Server pinging
		BalancedServerReference* m_balancedServersToPing = nullptr;
		int m_balancedServersToPingTotal = 0;
		int m_balancedServersToPingCurrentReceiving = -1;
		
		_ENetHost* m_ENetBalancedServerPingClient = nullptr;
		_ENetPeer* m_ENetBalancedServerPingPeer = nullptr;
		bool pingConnected = false;
		bool isPingingBalancedServers = false;
		const int pingsToSend = 10;
		int pingServerCurrent = 0;
		int pingsSent = 0;
		int pingsReceived = 0;
		int leastResponseTime = INT_MAX;
		int leastResponseIndex = 0;

		string	m_balancedServerName = "";
		string	m_balancedServerAddress = "";
		int		m_balancedServerPort = 0;
		bool	m_balancedServerReceived = false;
		bool	shouldConnectToBalancedServer = false;

		_ENetHost* m_ENetBalancedServerClient = nullptr;
		_ENetPeer* m_ENetBalancedServerPeer = nullptr;

		string	m_gameSessionName = "";
		string	m_gameSessionAddress = "";
		int		m_gameSessionPort = 0;
		bool	m_gameSessionReceived = false;
		bool	shouldConnectToGameSession = false;
	public:
		_ENetHost* m_ENetGameSessionClient = nullptr;
		_ENetPeer* m_ENetGameSessionPeer = nullptr;
	protected:
		bool isConnectedMasterServer = false;
		bool isConnectedBalancedServer = false;
		bool isConnectedGameSession = false;

	public:
		// Initialises ENet
		void Initialise();
		// Sets the address of the master server to connect to. Your client interface or configuration file should use this.
		void SetMasterServer(string address, unsigned int port);
		// Connect to the Master Server.
		void ConnectToMasterServer();
		// This is called internally after a response from the master server, but you can call it too.
		void SetBalancedServer(string address, unsigned int port, string name);
		// This is called internally after a response from the master server - but you can call it too.
		void ConnectToBalancedServer();

		// Sets the Game Session to connect to. Uses a packet struct.
		void SetGameSession(JNet::BalancedServerGameSessionInfo GSInfo);
		// This is called if a ConnectToGameSession packet is received, but you can call it too
		void ConnectToGameSession();

		// Main update loop
		void Update();

		// You can call this to get the Balanced Server to send you a packet for each Game Session.
		void RequestGameSessionsFromBalancedServer();
		// Request the Balanced Server find you a game session.
		void RequestFindGameSession();

		// Cleanly disconnect from a Game Session.
		void GameSessionDisconnect();
	
	protected:
		void UpdateMasterServer();
		void UpdateBalancedServer();
		void UpdateGameSession();

		void ProcessBalancedServerPinging();

	public:
		bool IsConnectedMasterServer() { return isConnectedMasterServer; }
		bool IsConnectedBalancedServer() { return isConnectedBalancedServer; }
		bool IsConnectedGameSession() { return isConnectedGameSession; }

		string GetBalancedServerName() { return m_balancedServerName; };
		string GetGameSessionName() { return m_gameSessionName; };
	};
}