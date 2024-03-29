#pragma once
#include <string>

struct _ENetHost;
struct _ENetPeer;
struct _ENetEvent;

using std::string;

namespace JNet
{
	class GameSession
	{
	public:
		GameSession();
		~GameSession();

	private:
		_ENetHost* m_ENetBalancedServerClient = nullptr;
		_ENetPeer* m_ENetBalancedServerPeer = nullptr;
		string m_balancedServerAddress = "";
		unsigned short m_balancedServerPort = 0;
	
	public:
		_ENetHost* m_ENetGameSessionClient = nullptr;
		void (*m_ClientConnectCallBack)(_ENetEvent*) = nullptr;
		void (*m_ClientPacketCallBack)(_ENetEvent*) = nullptr;
		void (*m_ClientDisconnectCallBack)(_ENetEvent*) = nullptr;

		void (*m_BalancedServerConnectCallBack)(_ENetEvent*) = nullptr;
		void (*m_BalancedServerPacketCallBack)(_ENetEvent*) = nullptr;
		void (*m_BalancedServerDisconnectCallBack)(_ENetEvent*) = nullptr;

	private:
		string m_myName = "";
		string m_myAddress = "";
		unsigned short m_myPort = 0;

	public:
		void Initialise();
		void Process();

		void SetBalancedServer(string address, unsigned int port);
		void SetMyConnectionInfo(string name, string address, unsigned int port);
		void ConnectToBalancedServer();

		void CheckInWithBalancedServer();
		void UpdateBalancedServer();

		void StartMyHost();
	};
}

