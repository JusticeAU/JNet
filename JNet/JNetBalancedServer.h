#pragma once

#include <vector>
#include <string>

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
			char name[32];
		};
	private:
		_ENetHost* m_ENetMasterServerClient;
		_ENetPeer* m_ENetMasterServerPeer;
		string m_masterServerAddress;
		unsigned short m_masterServerPort;
		bool m_connectedToMasterServer = false;
		bool m_openForConnections = false;

		string m_myName;
		string m_myAddress;
		unsigned short m_myPort;

		_ENetHost* m_ENetBalancedServerClient;
		vector<ConnectedPeer> m_connectedPeers;

		unsigned int m_playerCount;
		unsigned int m_playerCapacity;
		unsigned int m_sessionCount;

	public:
		void Initialise();
		void SetMasterServer(string address, unsigned int port);
		void SetMyConnectionInfo(string myName, string myAddress, unsigned int myPort);
		void ConnectToMasterServer();
		void OpenForConnections();
		void Run();
	};
}

