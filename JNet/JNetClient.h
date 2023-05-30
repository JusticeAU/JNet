#pragma once

#include <string>

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
			string address;
			unsigned short port;
		};

		struct BalancedServerReference
		{
			string name;
			string address;
			unsigned short port;
		};

		struct GameSessionReference
		{
			string name;
			string address;
			unsigned short port;
		};

	public:
		Client();
		~Client();

		void (*m_MasterServerConnectCallBack)(_ENetEvent*) = nullptr;
		void (*m_MasterServerPacketCallBack)(_ENetEvent*) = nullptr;
		void (*m_MasterServerDisconnectCallBack)(_ENetEvent*) = nullptr;

		void (*m_BalancedServerConnectCallBack)(_ENetEvent*) = nullptr;
		void (*m_BalancedServerPacketCallBack)(_ENetEvent*) = nullptr;
		void (*m_BalancedServerDisconnectCallBack)(_ENetEvent*) = nullptr;

		void (*m_ClientConnectCallBack)(_ENetEvent*) = nullptr;
		void (*m_ClientPacketCallBack)(_ENetEvent*) = nullptr;
		void (*m_ClientDisconnectCallBack)(_ENetEvent*) = nullptr;

	private:
		string	m_masterServerAddress;
		int		m_masterServerPort;

		_ENetHost* m_ENetMasterServerClient;
		_ENetPeer* m_ENetMasterServerPeer;
		bool m_shouldCloseMasterServerHost = false;

		// Balanced Server pinging
		BalancedServerReference* m_balancedServersToPing = nullptr;
		int m_balancedServersToPingTotal = 0;
		int m_balancedServersToPingCurrentReceiving = -1;
		
		_ENetHost* m_ENetBalancedServerPingClient;
		_ENetPeer* m_ENetBalancedServerPingPeer;
		bool pingConnected = false;
		bool isPingingBalancedServers = false;
		const int pingsToSend = 10;
		int pingServerCurrent = 0;
		int pingsSent = 0;
		int pingsReceived = 0;
		int leastResponseTime = INT_MAX;
		int leastResponseIndex;

		string	m_balancedServerName;
		string	m_balancedServerAddress;
		int		m_balancedServerPort;
		bool	m_balancedServerReceived = false;
		bool	shouldConnectToBalancedServer = false;

		_ENetHost* m_ENetBalancedServerClient;
		_ENetPeer* m_ENetBalancedServerPeer;

		string	m_gameSessionName;
		string	m_gameSessionAddress;
		int		m_gameSessionPort;
		bool	m_gameSessionReceived = false;
		bool	shouldConnectToGameSession = false;
	public:
		_ENetHost* m_ENetGameSessionClient;
		_ENetPeer* m_ENetGameSessionPeer;
	protected:
		bool isConnectedMasterServer = false;
		bool isConnectedBalancedServer = false;
		bool isConnectedGameSession = false;

	public:
		void Initialise();
		void SetMasterServer(string address, unsigned int port);
		void ConnectToMasterServer();
		void SetBalancedServer(string address, unsigned int port, string name);
		void ConnectToBalancedServer();
		void ConnectToGameSession();

		void Update();
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