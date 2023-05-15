#pragma once
#include <string>

struct _ENetHost;
struct _ENetPeer;

using std::string;

namespace JNet
{
	class GameSession
	{
	public:
		GameSession();
		~GameSession();

	private:
		_ENetHost* m_ENetBalancedServerClient;
		_ENetPeer* m_ENetBalancedServerPeer;
		string m_balancedServerAddress;
		unsigned short m_balancedServerPort;

		_ENetHost* m_ENetGameSessionClient;

		string m_myName;
		string m_myAddress;
		unsigned short m_myPort;

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

