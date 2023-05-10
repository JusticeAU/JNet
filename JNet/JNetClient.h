#pragma once

#include <string>

using std::string;

struct _ENetHost;
struct _ENetPeer;

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
			string adddress;
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


	private:
		string	m_masterServerAddress;
		int		m_masterServerPort;

		_ENetHost* m_ENetMasterServerClient;
		_ENetPeer* m_ENetMasterServerPeer;
		bool m_shouldCloseMasterServerHost = false;

		string	m_balancedServerName;
		string	m_balancedServerAddress;
		int		m_balancedServerPort;
		bool	m_balancedServerReceived = false;
		bool	shouldConnectToBalancedServer = false;

		_ENetHost* m_ENetBalancedServerClient;
		_ENetPeer* m_ENetBalancedServerPeer;

		bool isConnectedMasterServer = false;
		bool isConnectedBalancedServer = false;
		bool isConectedGameSession = false;

	public:
		void Initialise();
		void SetMasterServer(string address, unsigned int port);
		void ConnectToMasterServer();
		void SetBalancedrServer(string address, unsigned int port);
		void ConnectToBalancedServer();
		void ConnectToGameSession();

		void Update();
	protected:
		void UpdateMasterServer();
		void UpdateBalancedServer();
		void UpdateGameSession();
	};
}